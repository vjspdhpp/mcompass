import math

def elastic_interpolation(start, end, steps, k=0.1, rise=True):
    result = []
    for i in range(steps):
        t = i / (steps - 1)  # 将 t 归一化到 [0, 1]
        if rise:
            # 上升公式
            value = start + (end - start) * (1 - math.exp(-k * t))
        else:
            # 回落公式
            value = end + (start - end) * math.exp(-k * t)
        result.append(value)
    return result

# 从 270 到 450 的上升过程
rise_values = elastic_interpolation(270, 450, 45, k=0.1, rise=True)

# 从 450 到 360 的回落过程
fall_values = elastic_interpolation(450, 360, 15, k=0.1, rise=False)

# 合并两个序列
result = rise_values + fall_values

# 打印结果
for i, value in enumerate(result):
    print(f"Step {i + 1}: {value:.2f}")