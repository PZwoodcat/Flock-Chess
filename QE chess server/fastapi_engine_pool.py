# fastapi_engine_pool.py
# This is the recommended architecture for hosting a UCI chess engine pool
# For prototyping we will use a single engine for now. 
# In production, might use this but it is better to run in the browser.
import asyncio
import uuid
import hashlib
from typing import Optional, Dict, Any
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException
from pydantic import BaseModel

# ---- Config ----
ENGINE_CMD = ["./my_engine", "--uci"]  # replace with your engine command
POOL_SIZE = 4                          # tune based on memory/cpu
ENGINE_STARTUP_TIMEOUT = 10.0          # seconds
JOB_TIMEOUT = 60.0                     # per-job default timeout (seconds)

# ---- Job model ----
class Job(BaseModel):
    job_id: str
    user_id: Optional[str]
    fen: str
    params: Dict[str, Any] = {}
    status: str = "queued"  # queued, running, done, cancelled, error
    result: Optional[Dict[str, Any]] = None
    error: Optional[str] = None

# ---- Engine Process wrapper ----
class EngineProcess:
    def __init__(self, name: str):
        self.name = name
        self.proc: Optional[asyncio.SubprocessProtocol] = None
        self.queue: asyncio.Queue[Job] = asyncio.Queue()
        self.lock = asyncio.Lock()
        self._task: Optional[asyncio.Task] = None
        self.alive = False

    async def start(self):
        # Start the engine subprocess
        self.proc = await asyncio.create_subprocess_exec(
            *ENGINE_CMD,
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        self.alive = True
        # Start worker to consume queue
        self._task = asyncio.create_task(self._worker_loop())

        # Send UCI and wait for "uciok" (simplified)
        await self._send_cmd("uci")
        # Wait a short time for readiness; production parse "uciok"
        await asyncio.sleep(0.1)

    async def stop(self):
        self.alive = False
        if self._task:
            self._task.cancel()
        if self.proc:
            try:
                self.proc.stdin.write(b"quit\n")
                await self.proc.stdin.drain()
            except Exception:
                pass
            try:
                await asyncio.wait_for(self.proc.wait(), timeout=2)
            except Exception:
                self.proc.kill()
        self.proc = None

    async def _send_cmd(self, cmd: str):
        if not self.proc or not self.proc.stdin:
            raise RuntimeError("Engine not started")
        self.proc.stdin.write((cmd + "\n").encode())
        await self.proc.stdin.drain()

    async def _read_line(self) -> Optional[str]:
        if not self.proc or not self.proc.stdout:
            return None
        line = await self.proc.stdout.readline()
        if not line:
            return None
        return line.decode(errors="ignore").rstrip("\n")

    async def _worker_loop(self):
        while self.alive:
            job: Job = await self.queue.get()
            job.status = "running"
            try:
                # Example UCI conversation: set position, go depth/time
                await self._send_cmd(f"position fen {job.fen}")
                # Build go command from params
                go_cmd = "go"
                depth = job.params.get("depth")
                movetime = job.params.get("movetime")
                if depth:
                    go_cmd += f" depth {int(depth)}"
                elif movetime:
                    go_cmd += f" movetime {int(movetime)}"
                else:
                    go_cmd += " depth 12"
                await self._send_cmd(go_cmd)

                # Simple read loop to parse bestmove and info lines
                bestmove = None
                info_lines = []
                start = asyncio.get_event_loop().time()
                while True:
                    # timeout guard
                    if asyncio.get_event_loop().time() - start > (job.params.get("timeout", JOB_TIMEOUT) + 5):
                        raise TimeoutError("Engine response timeout")

                    line = await self._read_line()
                    if line is None:
                        raise RuntimeError("Engine terminated")
                    # For production, parse 'info ...' lines properly and stream them
                    if line.startswith("info"):
                        info_lines.append(line)
                        # Optionally: push partial progress to users via a pubsub / websocket
                    if line.startswith("bestmove"):
                        parts = line.split()
                        if len(parts) >= 2:
                            bestmove = parts[1]
                        break

                job.status = "done"
                job.result = {"bestmove": bestmove, "info": info_lines}
            except Exception as e:
                job.status = "error"
                job.error = str(e)
            finally:
                self.queue.task_done()

# ---- Engine Pool ----
class EnginePool:
    def __init__(self, size: int):
        self.engines = [EngineProcess(name=f"engine-{i}") for i in range(size)]
        self.job_store: Dict[str, Job] = {}  # in-memory; use Redis/DB in prod

    async def start_all(self):
        await asyncio.gather(*(e.start() for e in self.engines))

    async def stop_all(self):
        await asyncio.gather(*(e.stop() for e in self.engines))

    def _least_loaded_engine(self) -> EngineProcess:
        return min(self.engines, key=lambda e: e.queue.qsize())

    def _sticky_engine(self, user_id: str) -> EngineProcess:
        # simple consistent-ish hashing: prefer engine for session affinity
        idx = int(hashlib.sha256(user_id.encode()).hexdigest(), 16) % len(self.engines)
        chosen = self.engines[idx]
        # fallback: if chosen is overloaded, use least_loaded
        if chosen.queue.qsize() > 1:  # threshold
            return self._least_loaded_engine()
        return chosen

    async def submit_job(self, fen: str, user_id: Optional[str], params: Dict[str, Any]) -> str:
        job_id = str(uuid.uuid4())
        job = Job(job_id=job_id, user_id=user_id, fen=fen, params=params)
        self.job_store[job_id] = job
        # choose engine
        if user_id:
            engine = self._sticky_engine(user_id)
        else:
            engine = self._least_loaded_engine()
        await engine.queue.put(job)
        return job_id

    def get_job(self, job_id: str) -> Job:
        job = self.job_store.get(job_id)
        if not job:
            raise KeyError("job not found")
        return job

# ---- FastAPI wiring ----
app = FastAPI()
pool = EnginePool(size=POOL_SIZE)

@app.on_event("startup")
async def startup_event():
    await pool.start_all()

@app.on_event("shutdown")
async def shutdown_event():
    await pool.stop_all()

class SubmitRequest(BaseModel):
    fen: str
    user_id: Optional[str] = None
    depth: Optional[int] = 12
    movetime: Optional[int] = None
    timeout: Optional[int] = JOB_TIMEOUT

@app.post("/submit")
async def submit(req: SubmitRequest):
    params = {"depth": req.depth, "movetime": req.movetime, "timeout": req.timeout}
    job_id = await pool.submit_job(fen=req.fen, user_id=req.user_id, params=params)
    return {"job_id": job_id}

@app.get("/status/{job_id}")
async def status(job_id: str):
    try:
        job = pool.get_job(job_id)
    except KeyError:
        raise HTTPException(status_code=404, detail="job not found")
    return {"job_id": job.job_id, "status": job.status, "result": job.result, "error": job.error}

# Simple WebSocket streaming for job updates (very basic)
# In production, use pubsub, Redis, or notify mechanism so engine worker can push updates.
@app.websocket("/ws/jobs/{job_id}")
async def ws_job(websocket: WebSocket, job_id: str):
    await websocket.accept()
    try:
        while True:
            try:
                job = pool.get_job(job_id)
            except KeyError:
                await websocket.send_json({"error": "job not found"})
                await websocket.close()
                return

            await websocket.send_json({"job_id": job.job_id, "status": job.status, "result": job.result, "error": job.error})
            # If finished, close
            if job.status in ("done", "error", "cancelled"):
                await websocket.close()
                return
            await asyncio.sleep(0.5)
    except WebSocketDisconnect:
        return
