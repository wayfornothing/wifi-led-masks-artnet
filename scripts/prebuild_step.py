import os
import subprocess
import platform
import shutil

# generate version file
system = platform.system()

if system == "Windows":
    cwd = os.getcwd()
    script_path = os.path.join(cwd, "scripts", "gen_version.ps1")
    pwsh = shutil.which("pwsh") or shutil.which("powershell")
    if not pwsh:
        raise RuntimeError("PowerShell is required on Windows but not found.")
    subprocess.run(
        [pwsh, "-ExecutionPolicy", "Bypass", "-File", script_path],
        check=True
    )
else:
    script_path = os.path.join(os.getcwd(), "scripts/gen_version.sh")
    subprocess.run(["bash", script_path], check=True)


# copy html files into progmem
DATA_DIR = "data"
HEADER_DIR = "include"
files = [f for f in os.listdir(DATA_DIR) if f.endswith(".html")]
for file in files:
    HTML_FILE = os.path.join(DATA_DIR, file)
    HTML_HEADER = os.path.join(HEADER_DIR, file + ".h")
    HTML_VAR = file.replace(".", "_").upper()
    if not os.path.exists(HTML_HEADER):
        print(HTML_FILE)
        with open(HTML_HEADER, "w", encoding="utf-8") as header, open(HTML_FILE, "r", encoding="utf-8") as html:
            code ="""
#pragma once

#include <Arduino.h>

const char __VAR__[] PROGMEM = R"rawliteral(
"""
            code = code.replace("__VAR__", HTML_VAR)
            header.write(code)
            header.write(html.read())
            code = """
)rawliteral";
"""
            header.write(code)
            header.close()
            html.close()
