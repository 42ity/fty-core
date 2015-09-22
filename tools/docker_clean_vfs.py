#!/usr/bin/env python
"""
Copyright (C) 2014 Adam Ever-Hadani

Note (Arnaud Quette): here is the answer of the author on the license
clarification request
	Glad you found it useful, by all means use away. If u scroll down
	that thread ull see I posted it as a public gist as well which I
	would assume implies some default open source license. Otherwise
	consider this a release from any copyright claims :)

For now at least, we will use APLv2, to be coherent with Docker:

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

From https://github.com/docker/docker/issues/6354#issuecomment-60817733

 \file    docker_clean_vfs.py
 \brief   Check and purge all existing Docker containers for zombie directories
 \author  Adam Ever-Hadani <https://github.com/adamhadani>
 \details Check all existing Docker containers for their mapped paths, and
          then purge any zombie directories in docker's volumes directory
          which don't correspond to an existing container.

"""

"""
FAQ (added by Eaton)
when using this script ? 
when realize that Docker is still taking a big chunk of your HD

Howto do a diagnostic ?
du -h -d1 /var/lib/docker/vfs/dir 

howto install ?
apt-get install python-pip
pip install docker-py

howto use it ?
sudo python docker_clean_vfs.py
"""
import logging
import os
import sys
from shutil import rmtree

import docker

DOCKER_VOLUMES_DIR = "/var/lib/docker/vfs/dir"


def get_immediate_subdirectories(a_dir):
    return [os.path.join(a_dir, name) for name in os.listdir(a_dir)
            if os.path.isdir(os.path.join(a_dir, name))]


def main():
    logging.basicConfig(level=logging.INFO)

    client = docker.Client(version='1.15')


    valid_dirs = []
    for container in client.containers(all=True):
        volumes = client.inspect_container(container['Id'])['Volumes']
        if not volumes:
            continue

        for _, real_path in volumes.iteritems():
            if real_path.startswith(DOCKER_VOLUMES_DIR):
                valid_dirs.append(real_path)

    all_dirs = get_immediate_subdirectories(DOCKER_VOLUMES_DIR)
    invalid_dirs = set(all_dirs).difference(valid_dirs)

    logging.info("Purging %s dangling Docker volumes out of %s total volumes found.",
                 len(invalid_dirs), len(all_dirs))
    for invalid_dir in invalid_dirs:
        logging.info("Purging directory: %s", invalid_dir)
        rmtree(invalid_dir)

    logging.info("All done.")


if __name__ == "__main__":
    sys.exit(main())
