#!/usr/bin/python3

import os
import subprocess

SCRIPT_DIRECTORY = os.path.dirname(__file__) + "\\"
BOOTSTRAPPING_REPO_HTTPS_ADDR = "https://github.com/corporateshark/bootstrapping.git"
BOOTSTRAPPING_VERSION = "f395ada"

def setup_boostrapping():
   if not os.path.isdir(SCRIPT_DIRECTORY + "..\\libraries\\bootstrapping"):
      # path is different when cloning to account for the directory changes below (seems like it doesn't get fully evaluated until it's used by the subprocess)
      command_clone = ["git", "clone", BOOTSTRAPPING_REPO_HTTPS_ADDR, os.path.join("bootstrapping")]
      command_branch = ["git", "branch", "setup-commit", BOOTSTRAPPING_VERSION]

      os.chdir("libraries")
      # clone boostrapping
      subprocess.run(command_clone, check=True, capture_output=True, text=True)
      print("Bootstrapping repository cloned. Checking out specified commit.")
      os.chdir("bootstrapping")
      # checkout this specific version of boostrapping expected
      subprocess.run(command_branch)
      print("Commit checked out.")
   else:
      print("Bootstrapping found, skipping boostrapping setup")

def run_boostrapping():
   print("Bootstrapping Starting...")
   libraries_location = SCRIPT_DIRECTORY + "..\\libraries"
   bootstrapping_location = libraries_location + "\\bootstrapping"
   script = bootstrapping_location + "\\bootstrap.py"
   json = libraries_location + "\\bootstrap.json"

   command = ['python3', script, '-b', libraries_location, '--local-bootstrap-file=' + json, '--break-on-first-error']
   result = subprocess.check_output(command, shell=True, text=True)
   print(result)
   print("Boostrapping Finished.")

def compile_shaders():
   print("Shader compilation Starting...")
   command = ['call', SCRIPT_DIRECTORY + 'compile-shaders.bat']
   result = subprocess.check_output(command, shell=True, text=True)
   print(result)
   print("Shader compilation Finished.")


print("Beginning engine setup.")
setup_boostrapping()
run_boostrapping()
compile_shaders()
