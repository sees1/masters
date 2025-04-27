#!/usr/bin/env python3

import os
import sys
from pathlib import Path
import typing
import subprocess as sp
from colorama import init as colorama_init
from colorama import Fore
import re
import signal
import json
import lib.common as cmn

colorama_init()

os.environ['U_ID'] = sp.getoutput("echo ${U_ID:-`id -u`}")
os.environ['G_ID'] = sp.getoutput("echo ${G_ID:-`id -g`}")
os.environ['ARCH'] = sp.getoutput("echo ${ARCH:-`uname -m`}")
os.environ['TAG'] = "local"
os.environ['NUM_THREADS'] = sp.getoutput("echo ${NUM_THREADS:-`nproc`}")

ARCH = os.getenv('ARCH')

def get_groups() -> typing.List[str]:
    domain_dir = cmn.SUPER_DIR + "/build"
    if not os.path.isdir(domain_dir):
        return []
    return [d.name for d in Path(domain_dir).iterdir() if d.is_dir() and os.path.isfile(d.as_posix() + "/docker-compose.yml")]

def get_image_name_in_compose(group: str, component: str) -> typing.Dict:
    group_yml = '/'.join([cmn.SUPER_DIR, 'build', group, 'docker-compose.yml'])
    if not os.path.isfile(group_yml):
        return ''
    cmd = ['docker', 'compose', '-f', group_yml, 'config', component, '--images']
    str_out = sp.check_output(cmd, universal_newlines=True,stderr=sp.DEVNULL ).strip()   
    match = re.search(r"^(?P<ros_prefix>.+)\/(?P<image_name>.+):(?P<tag>.+)$", str_out)  
    return match.groupdict()

def print_groups_and_components_in_json():
    out_dict = {}
    for group in get_groups():
        out_dict[group] = list(cmn.parse_build_services(group).keys())
    print(json.dumps(out_dict))

def help():
    print(f"{Fore.GREEN}Usage: build.py -g=[GROUPS] -c=[COMPONENTS]{Fore.RESET}")
    print()
    print("Build Util Options:")
    print("     -g=<string>           Functional group list, ex: -g=planning,utils")
    print("     -c=<string>           Components list, ex: -c=pnc,rc")
    print("     --get-image-name=<group:string>,<component:string>    Return json of image name for componet in group")
    print("     --list-json           List groups and components in json")
    print("     --with-base           Build base images first")
    print("     --discover            Print discover info and exit")
    print("     --repos-update        git submodule update and lfs pull for x-repos")
    print("     --attempts            Attempts to build if dockerfile build fails. default=1")
    print()
    print("Available groups and components for arch", cmn.get_arch_name())
    for group in get_groups():
        print(f"{Fore.YELLOW}" + group + f":{Fore.RESET}")
        print("  " + ", ".join(list(cmn.parse_build_services(group).keys())))

def build_group(group: str, components: typing.List[str], build_attempts: int):
    print('build group', f"{Fore.YELLOW}", group, f"{Fore.RESET}", 'with components', f"{Fore.YELLOW}", components, f"{Fore.RESET}")
    os.chdir('/'.join([cmn.SUPER_DIR, 'build', group]))
    docker_cmd = ["docker", "compose", "build"] + components
    attempt = build_attempts
    while attempt > 0:
        attempt = attempt - 1
        completed_process = sp.run(docker_cmd)
        if completed_process.returncode == 0:
            break
        print(f"{Fore.RED}subprocess \"{' '.join(docker_cmd)}\" failed: return code {completed_process.returncode}{Fore.RESET}")
        if attempt <= 0:
            print(f"{Fore.RED}build failed for all {build_attempts} attempts. exit.{Fore.RESET}")
            exit(1)
        else:
            print(f"{Fore.RED}build attempts left: {attempt}{Fore.RESET}")

class BuildingTaskInfo:
    """ Building task info"""
    # task dict for building.
    # key: groups
    # values: list of component names
    build_task = dict()
    # list of components in base group, that we shall build
    base_components = set()
    # component list, that not found in all groups
    unknown_components = set()
    # unknown group list
    unknown_groups = []
    # unknown base list
    unknown_base = set()
    repos = {}
    # print info
    def print(self):
        print("build_task:", self.build_task)
        print("base_components:", self.base_components)
        print("unknown_components:", self.unknown_components)
        print("unknown_groups:", self.unknown_groups)
        print("unknown_base:", self.unknown_base)
        print("repos:", self.repos)

def discover(groups: typing.List[str], components: typing.List[str]) -> BuildingTaskInfo:
    info = BuildingTaskInfo()
    info.unknown_components = set(components)
    info.base_components = set()
    info.unknown_groups = []
    info.repos = set()
    existing_groups = get_groups()
    known_base = set()
    if "base" in existing_groups:
        known_base = set(cmn.parse_build_services("base").keys())
    for group in groups:
        if group in existing_groups:
            parsed_services = cmn.parse_build_services(group)
            services = []
            if not components:
                services = list(parsed_services.keys())
            else:
                services = set(parsed_services.keys()).intersection(set(components))
            if services:
                info.build_task[group] = list(services)
                info.unknown_components = info.unknown_components.difference(services)
                base_components = set(parsed_services[s]["base"] for s in services if s in parsed_services and parsed_services[s]["base"])
                info.base_components = info.base_components.union(base_components)
                info.unknown_base = info.unknown_base.union(base_components.difference(known_base))
                for repos in [parsed_services[s]["x-repos"] for s in services if s in parsed_services]:
                    for repo in repos:
                        info.repos.add(repo)
        else:
            info.unknown_groups.append(group)
    return info

def build_groups(groups: typing.List[str], components: typing.List[str], is_build_with_base: bool, is_repos_update: bool, build_attempts: int):
    print("build for", f"{Fore.YELLOW}" + ARCH + f"{Fore.RESET}", "aka", f"{Fore.YELLOW}" + cmn.get_arch_name() + f"{Fore.RESET}")

    build_info = discover(groups, components)
    if build_info.unknown_groups:
        print(f"{Fore.RED}" + "groups", "'" + ", ".join(build_info.unknown_groups) + "'", "does not not exist", f"{Fore.RESET}")
        exit(1)
    if build_info.unknown_components:
        print(f"{Fore.RED}" + "components", "'" + ", ".join(build_info.unknown_components) + "'" ,"not found in specified groups", f"{Fore.RESET}")
        exit(1)
    if build_info.unknown_base:
        print(f"{Fore.RED}" + "components", "'" + ", ".join(build_info.unknown_base) + "'" ,"not found in group 'base'", f"{Fore.RESET}")
        exit(1)

    if is_repos_update:
        for repo in build_info.repos:
            cmd = ["git", "-C", cmn.SUPER_DIR, "submodule", "update", "--recursive", "--init", repo]
            print(f"{Fore.GREEN}repos update " + repo + f"{Fore.RESET}")
            print(cmd)
            if sp.run(cmd).returncode != 0:
                print(f"{Fore.RED}" + "can't update submodule " + repo + f"{Fore.RESET}")
                exit(1)
            cmd = ["git", "-C", "/".join([cmn.SUPER_DIR, repo]), "lfs", "pull"]
            print(cmd)
            if sp.run(cmd).returncode != 0:
                print(f"{Fore.RED}" + "can't git lfs pull " + repo + f"{Fore.RESET}")
                exit(1)
        exit(0)

    # build base
    if is_build_with_base and build_info.base_components:
        print(f"{Fore.YELLOW}build base components: " + ", ".join(build_info.base_components) + f"{Fore.RESET}")
        build_group("base", list(build_info.base_components), build_attempts)

    # build groups
    for group in build_info.build_task.keys():
        print(f"{Fore.YELLOW}build " + group + " components: " + ", ".join(build_info.build_task[group]) + f"{Fore.RESET}")
        build_group(group, build_info.build_task[group], build_attempts)

def handler(signum, frame):
    print("Ctrl-C")
    # ???
    exit(1)

signal.signal(signal.SIGINT, handler)

def main():
    groups = []
    components = []
    is_build_with_base = False
    is_discover_and_exit = False
    is_repos_update = False
    build_attempts = 1

    for arg in sys.argv[1:]:
        if arg.startswith("--help") or arg.startswith("-h"):
            help()
            exit(0)
        if arg.startswith("-g"):
            groups += filter(None, arg[len("-g="):].split(","))
        elif arg.startswith("-c"):
            components += filter(None, arg[len("-c="):].split(","))
        elif arg.startswith("--attempts="):
            build_attempts = int(arg[len("--attempts="):])
        elif arg.startswith("--get-image-name"):
            params = []
            params += filter(None, arg[len("--get-image-name="):].split(","))            
            if len(params) == 2:
                group = params[0]
                component = params[1]
                print(json.dumps(get_image_name_in_compose(group,component)))
            else:
                print("--get-image-name=<group:string>,<component:string>")
            exit(0)
        elif arg == '--list-json':
            print_groups_and_components_in_json()
            exit(0)
        elif arg == '--with-base':
            is_build_with_base = True
        elif arg == '--discover':
            is_discover_and_exit = True
        elif arg == '--repos-update':
            is_repos_update = True
        else:
            print("unrecognized arg: " + arg)
            help()
            exit(1)

    if not groups:
        groups = get_groups()

    if is_discover_and_exit:
        info = discover(groups, components)
        info.print()
        exit(0)

    build_groups(groups, components, is_build_with_base, is_repos_update, build_attempts)

if __name__ == '__main__':
    main()
