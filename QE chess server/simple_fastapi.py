import subprocess
from fastapi import FastAPI
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware
from pathlib import Path

# get current script directory
here = Path(__file__).resolve().parent
root = here.parent
# append the path in a platform-independent way
exe_path = root / "Flock Chess - public" / "build" / "src" / "Debug" / "entry.exe"
# print("Using executable:", exe_path)

app = FastAPI()

# allow local dev origins (or use ["*"] for quick dev)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:8080", "http://127.0.0.1:8080", 
                   "http://localhost:5500", "http://127.0.0.1:5500"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class MultiplyInput(BaseModel):
    a: int
    b: int

@app.post("/compute")
def compute(data: MultiplyInput):
    result = subprocess.run(
        [exe_path, str(data.a), str(data.b)],
        capture_output=True,
        text=True
    )
    return {"output": result.stdout.strip()}
