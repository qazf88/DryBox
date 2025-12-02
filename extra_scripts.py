Import("env")
import shutil
import os

PRFIX_NAME = "drybox_"

def rename_and_copy(source, target, env):
    firmware_path = target[0].get_abspath()
    config = env.GetProjectConfig()

    major = config.get("version", "major")
    minor = config.get("version", "minor")
    patch = config.get("version", "patch")

    new_name = f"{PRFIX_NAME}{major}.{minor}.{patch}.bin"

    project_dir = env["PROJECT_DIR"]
    firmware_dir = os.path.join(project_dir, "firmware")
    os.makedirs(firmware_dir, exist_ok=True)
    for f in os.listdir(firmware_dir):
        if f.startswith(PRFIX_NAME) and f.endswith(".bin"):
            try:
                os.remove(os.path.join(firmware_dir, f))
                print("Removed old file:", f)
            except Exception as e:
                print("Failed to remove:", f, e)    

    original_path = os.path.join(firmware_dir, "firmware.bin")
    shutil.copy(firmware_path, original_path)
    
    new_path = os.path.join(firmware_dir, new_name)
    if os.path.exists(new_path):
        os.remove(new_path)

    os.rename(original_path, new_path)

env.AddPostAction("$PROGPATH", rename_and_copy)