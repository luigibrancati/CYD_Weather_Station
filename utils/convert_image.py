from PIL import Image
import numpy as np
    
from pathlib import Path
path = Path(__file__).resolve().parent
img = Image.open(path / "wind.png")
ratio = 24 / img.size[0]
newimg = img.resize((int(img.size[0] * ratio), int(img.size[1] * ratio)), Image.Resampling.LANCZOS).quantize(colors=128, method=2)
newimg.save(path / "wind_resized.png", optimize=True, quality=100)
array = np.array(newimg)
print(array.shape)
print(array.dtype)
print(array)
converted_hex = np.apply_along_axis(lambda row: [f"0x{v:02x}" for v in row], 1, array)
for row in converted_hex:
    print(",".join(row))
