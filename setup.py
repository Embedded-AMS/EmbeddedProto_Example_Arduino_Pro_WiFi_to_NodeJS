#
# Copyright (C) 2020-2022 Embedded AMS B.V. - All Rights Reserved
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
#   Johan Huizingalaan 763a
#   1066 VH, Amsterdam
#   the Netherlands
#

import subprocess
import argparse
from os import chdir

# Update the submodule Embedded Proto before importing its setup script.
subprocess.run(["git", "submodule", "init"], check=True, capture_output=True)
subprocess.run(["git", "submodule", "update"], check=True, capture_output=True)

from EmbeddedProto import setup as EPSetup


####################################################################################

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="This script will setup the environment to generate Embedded Proto "
                                                 "code in your project.")

    parser.add_argument('-g', '--generate', action='store_true', 
                        help="Do not run the Embedded Proto setup. Only generate the source based on *.proto files. "
                             "Use this after changing weather.proto.")

    EPSetup.add_parser_arguments(parser)
    EPSetup.display_feedback()
    args = parser.parse_args()

    
    chdir("./EmbeddedProto")
    
    if not args.generate:
        # Run the setup script of Embedded Proto it self.
        EPSetup.run(args)

    # Generate the source file based on weather.proto
    subprocess.run(["protoc", "--plugin=protoc-gen-eams=protoc-gen-eams", "-I../Proto", "--eams_out=../ArduinoClient",
                    "../Proto/weather.proto"], check=True, capture_output=True)

    # If the script did not exit before here we have completed it with success.
    print("Setup completed with success!")
