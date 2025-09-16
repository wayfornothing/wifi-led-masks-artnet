import os
import subprocess

# gerenate version file
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
        with open(HTML_HEADER, "w") as header, open(HTML_FILE, "r") as html:

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
