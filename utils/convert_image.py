from PIL import Image
import numpy as np
from pathlib import Path
import json

path = Path(__file__).resolve().parent
with open(path / "weather_images.json", "r") as f:
    data = json.load(f)
for i in range(len(data)):
    data[i] = int(data[i], 16)
data = [data[i:i+4] for i in range(len(data) - 3)]
data = [data[i:i+24] for i in range(len(data) - 23)]
print(data)
img = Image.fromarray(np.array(data, dtype=np.uint8), mode="RGBA")
# img = img.resize((img.size[0] * 4, img.size[1] * 4), Image.Resampling.LANCZOS)
img.save(path / "test.png", optimize=True, quality=100)
# ratio = 24 / img.size[0]
# newimg = img.resize((int(img.size[0] * ratio), int(img.size[1] * ratio)), Image.Resampling.LANCZOS).quantize(colors=128, method=2)
# newimg.save(path / "wind_resized.png", optimize=True, quality=100)
# array = np.array(newimg)
# print(array.shape)
# print(array.dtype)
# print(array)
# converted_hex = np.apply_along_axis(lambda row: [f"0x{v:02x}" for v in row], 1, array)
# for row in converted_hex:
#     print(",".join(row))

