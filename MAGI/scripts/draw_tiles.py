import argparse
import pathlib

from PIL import Image

TILE_HEIGHT = 8
TILE_WIDTH = 8
TILE_LENGTH_BYTES = 16

TILES_PER_ROW = 8

COLORS = [
    (0x55, 0x38, 0x40),
    (0x9b, 0x68, 0x59),
    (0xbe, 0xbc, 0x6a),
    (0xed, 0xf8, 0xc8),
]

# tile data locs:
#   06:79a6 (1120 bytes) - ???
#   06:74d6 (720 bytes) - ???
#   06:77a6 (384 bytes) - mon stat names (str/def/spd/skill/res/luk)
#   06:7926 (128 bytes) - mon stat names 2 (lvl/exp)

parser = argparse.ArgumentParser()
parser.add_argument("--mode", default="text")
parser.add_argument("--tiles-per-row", default=TILES_PER_ROW, type=int)
parser.add_argument("file")
args = parser.parse_args()

raw_data = []
if args.mode == "text":
    with open(args.file, "r") as f:
        for line in f:
            line = line.replace("$", "").strip()
            if line == "":
                continue
            raw_data.extend([int(b, 16) for b in line.split(" ")])
else:
    with open(args.file, "rb") as f:
        while b := f.read(1):
            raw_data.append(b[0])

#assert len(raw_data) % TILE_LENGTH_BYTES == 0
num_tiles = len(raw_data) // TILE_LENGTH_BYTES
print(f"num_bytes={len(raw_data)}")
print(f"num_tiles={num_tiles}")

output = []
height_tiles = (num_tiles // args.tiles_per_row) + (0 if num_tiles % args.tiles_per_row == 0 else 1)
width_tiles = min(num_tiles, args.tiles_per_row)

print(f"width_tiles={width_tiles}")
print(f"height_tiles={height_tiles}")

for y in range(height_tiles * TILE_HEIGHT):
    for x in range(width_tiles * TILE_WIDTH):
        output.append((0, 0, 0))

print(f"output_size_bytes={len(output)}")
raw_colors = []
for tile_idx in range(num_tiles):
    # print(f"tile_idx={tile_idx}")
    for y in range(0, TILE_HEIGHT):
        tile_lo = raw_data[TILE_LENGTH_BYTES * tile_idx + 2*y]
        tile_hi = raw_data[TILE_LENGTH_BYTES * tile_idx + 2*y + 1]
        # print("y", y, tile_hi, tile_lo)
        for x in range(TILE_WIDTH):
            imgx = (tile_idx % args.tiles_per_row) * TILE_WIDTH + x
            imgy = (tile_idx // args.tiles_per_row) * TILE_HEIGHT + y
            idx = imgy * (width_tiles * TILE_WIDTH) + imgx
            shift = TILE_WIDTH - (x + 1)
            raw_color = (((tile_hi >> shift) & 1) << 1) | ((tile_lo >> shift) & 1)
            # raw_colors.append(raw_color)
            # print("x", imgx, imgy, idx, raw_color)
            output[idx] = COLORS[raw_color]
# print(raw_colors)

img = Image.new("RGB", (width_tiles*TILE_WIDTH, height_tiles*TILE_HEIGHT))
img.putdata(output)

outputfile = pathlib.Path(args.file).with_suffix(".png")
img.save(outputfile)
