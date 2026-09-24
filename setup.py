#
# Copyright (C) 2020-2026 Embedded AMS B.V. - All Rights Reserved
#
# This file is part of Embedded Proto.
#
# Embedded Proto is open source software: you can redistribute it and/or 
# modify it under the terms of the GNU General Public License as published 
# by the Free Software Foundation, version 3 of the license.
#
# Embedded Proto  is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Embedded Proto. If not, see <https://www.gnu.org/licenses/>.
#
# For commercial and closed source application please visit:
# <https://EmbeddedProto.com/license/>.
#
# Embedded AMS B.V.
# Info:
#   info at EmbeddedProto dot com
#
# Postal address:
#   Atoomweg 2
#   1627 LE, Hoorn
#   the Netherlands
#

import subprocess
import argparse
from os import chdir
import os
import platform

# Update the submodule Embedded Proto before importing its setup script.
subprocess.run(["git", "submodule", "init"], check=True, capture_output=True)
subprocess.run(["git", "submodule", "update"], check=True, capture_output=True)

from EmbeddedProto import install as EPInstall


####################################################################################

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="This script will setup the environment to generate Embedded Proto "
                                                 "code in your project.")

    parser.add_argument('-g', '--generate', action='store_true', 
                        help="Do not run the Embedded Proto install. Only generate the source based on *.proto files. "
                             "Use this after changing weather.proto.")

    EPInstall.add_parser_arguments(parser)
    EPInstall.display_feedback()
    args = parser.parse_args()

    
    if not args.generate:
        # Run the install script of Embedded Proto it self.
        chdir("./EmbeddedProto")
        EPInstall.run(args)
        chdir("..")

    # Generate the source file based on weather.proto. The embeddedproto command lives in the virtual environment of
    # the submodule. It runs the protobuf compiler which came with the package and adds the plugin for you.
    if "Windows" == platform.system():
        embeddedproto = os.path.join("EmbeddedProto", "venv", "Scripts", "embeddedproto.exe")
    else:
        embeddedproto = os.path.join("EmbeddedProto", "venv", "bin", "embeddedproto")
    subprocess.run([embeddedproto, "-I./Proto", "--eams_out=./ArduinoClient", "./Proto/weather.proto"], check=True)

    # If the script did not exit before here we have completed it with success.
    print("Setup completed with success!")
