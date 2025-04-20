#!/usr/bin/python3

import os
import sys
import subprocess

BOOTSTRAPPING_REPO_HTTPS_ADDR = "https://github.com/corporateshark/bootstrapping.git"
BOOTSTRAPPING_VERSION = "f395ada"

def setup_boostrapping():
   if not os.path.isdir(os.path.join("libraries\\bootstrapping")):
      # path is different when cloning to account for the directory changes below (seems like it doesn't get fully evaluated until it's used by the subprocess)
      command_clone = ["git", "clone", BOOTSTRAPPING_REPO_HTTPS_ADDR, os.path.join("bootstrapping")]
      command_branch = ["git", "branch", "setup-commit", BOOTSTRAPPING_VERSION]
      command_bootstrap = ["python3", "bootstrap.py"]

      os.chdir("libraries")
      # clone boostrapping
      subprocess.run(command_clone, check=True, capture_output=True, text=True)
      print("Bootstrapping repository cloned. Checking out specified commit.")
      os.chdir("bootstrapping")
      # checkout this specific version of boostrapping expected
      subprocess.run(command_branch)
      print("Commit checked out.")
      os.chdir("../../")
   else:
      print("Bootstrapping found, skipping boostrapping setup")

def run_boostrapping():
   libraries_location = "libraries"
   bootstrapping_location = os.path.join(libraries_location, "bootstrapping")
   script = os.path.join(bootstrapping_location, "bootstrap.py")
   json = os.path.join(libraries_location, "bootstrap.json")

   command = 'python3 {} -b {} --local-bootstrap-file={} --break-on-first-error'.format(script, libraries_location, json)
   os.system(command)


print("Beginning engine setup.")
setup_boostrapping()
run_boostrapping()
