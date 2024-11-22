import shutil
import os
import sys
import site
from loguru import logger as log

def main():
    log.debug("=== cwd: {}  args: {}", os.getcwd(), sys.argv[1:])
    proj_dir = sys.argv[1]
    targ_file = f"{proj_dir}/{sys.argv[2]}"

    build_dir = f"{proj_dir}/build"
    site_dir = [f for f in site.getsitepackages() if f.find('site-packages') > 0][0]
    
    inst_dir = f"{site_dir}/dearpygui"
    if not os.path.isdir(inst_dir):
        log.warning("Dearpygui not installed")
    else:
        shutil.copy(targ_file, inst_dir)
        shutil.copy(f"{proj_dir}/dearpygui/dearpygui.py", inst_dir)
        log.info("dearpygui install updated {}", inst_dir)
    shutil.copy(targ_file, f"{proj_dir}/dearpygui/")

if __name__ == '__main__':
    main()
