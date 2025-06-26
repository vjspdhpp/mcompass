Import("env")
import os
import subprocess

firmware_version = "1.1.0"

def get_git_info():
    try:
        # Get current branch name
        branch = subprocess.check_output(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"],
            universal_newlines=True
        ).strip()

        # Get current commit hash
        commit = subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"], 
            universal_newlines=True
        ).strip()

        return branch, commit
    except:
        return "unknown", "unknown"

print("==== ALL ENVIRONMENT VARIABLES ====")
for key, value in os.environ.items():
    print(f"{key}={value}")

# 读取环境变量设置型号和服务器模式
model = os.getenv("DEFAULT_MODEL", "LITE")          # 默认 "LITE"
server_mode = os.getenv("DEFAULT_SERVER_MODE", "BLE")  # 默认 "BLE"
sensor_model = os.getenv("DEFAULT_SENSOR_MODEL", "5883P")  # 默认 "5883P"



model_enum = f"mcompass::Model::{model.upper()}"
server_mode_enum = f"mcompass::ServerMode::{server_mode.upper()}"  # 生成服务器模式枚举
if sensor_model == "5883L":
    sensor_model_enum = 0
else:
    sensor_model_enum = 1

branch, commit = get_git_info()
env.Append(CPPDEFINES=[
    ("GIT_BRANCH", '\\"%s\\"' % branch),
    ("GIT_COMMIT", '\\"%s\\"' % commit),
    ("BUILD_VERSION", '\\"%s\\"' % firmware_version),
    ("DEFAULT_MODEL", model_enum),  # 覆盖宏定义中的默认型号
    ("DEFAULT_SERVER_MODE", server_mode_enum),  # 新增服务器模式定义
    ("DEFAULT_SENSOR_MODEL", sensor_model_enum)  # 新增磁力传感器型号定义
])
print(f"Model: {model}, Server Mode: {server_mode}, Sensor Model: {sensor_model}")
print(f"Model enum: {model_enum}, Server Mode enum: {server_mode_enum}, Sensor Model enum: {sensor_model_enum}")
