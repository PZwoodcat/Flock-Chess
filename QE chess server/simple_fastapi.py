import subprocess
from fastapi import FastAPI
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware
from pathlib import Path
import platform
import json

# get current script directory
here = Path(__file__).resolve().parent
root = here.parent
cross_platform_common_path = root / "Flock Chess - public" / "build" / "src" / "Debug"
# append the path in a platform-independent way
def get_executable_path(cross_platform_common_path, runner):
    """Return the correct executable path depending on the OS."""
    if platform.system() == "Windows":
        return cross_platform_common_path / (runner + ".exe")
    else:
        return cross_platform_common_path / runner
exe_path = get_executable_path(cross_platform_common_path, "entry")
exe_path_analyze = get_executable_path(cross_platform_common_path, "analyze_test")
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

@app.post("/multiply")
def compute(data: MultiplyInput):
    result = subprocess.run(
        [exe_path, str(data.a), str(data.b)],
        capture_output=True,
        text=True
    )
    return {"output": result.stdout.strip()}

class AnalyzeTest(BaseModel):
    a: str
    b: str

@app.post("/analyze_test")
def analyze_test(data: AnalyzeTest):
    result = subprocess.run(
        [exe_path_analyze, str(data.a), str(data.b)],
        capture_output=True,
        text=True
    )
    return json.loads(result.stdout)
