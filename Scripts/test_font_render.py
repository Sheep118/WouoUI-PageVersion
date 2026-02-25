"""
Standalone font render test using Pillow.
Edit FONT_PATH and TEST_CHAR as needed.
"""

from PIL import Image, ImageDraw, ImageFont
import numpy as np

# User-editable inputs
# FONT_PATH = r"../font/JinzisheTongfang-Regular/JinzisheTongfang-Regular.ttf"  # 英文 6*13 中文 12*13
# FONT_PATH = r"../font/Pixeloid_Font_1_0/TrueType (.ttf)/PixeloidMono.ttf"  # 英文 8*15 中文 不含中文
# FONT_PATH = r"../font/Terminus/TerminessNerdFont-Bold.ttf" # 英文 6*14 没有中文
# FONT_PATH = r"../font/unifont/unifont-16_0_04.ttf" # 英文 7*13 中文12*13
# FONT_PATH = r"../font/vonwaon-bitmap.ttf/VonwaonBitmap-12px.ttf" # 英文 8*12 中文12*12 ,这个还有个专门的16px版本
# FONT_PATH = r"../font/vonwaon-bitmap.ttf/VonwaonBitmap-16px.ttf" # 英文 8*16 中文16*16 
FONT_PATH = r"../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf" # 英文 6*12 中文12*12
TEST_CHAR = "hgZ"
FONT_SIZE = None  # Set to an int to force size; None uses font's embedded size if possible
THRESHOLD = 0


def main():
    if not FONT_PATH:
        raise SystemExit("Set FONT_PATH at the top of this file.")

    if FONT_SIZE is None:
        try:
            font = ImageFont.truetype(FONT_PATH, 0)
        except Exception:
            font = ImageFont.truetype(FONT_PATH, 12)
    else:
        font = ImageFont.truetype(FONT_PATH, FONT_SIZE)

    # Use font metrics to set a stable glyph box
    ascent, descent = font.getmetrics()
    base_height = max(1, ascent + descent)
    print("Ascent: {}, Descent: {}, Base height: {}".format(ascent, descent, base_height))

    if hasattr(font, "getlength"):
        base_width = max(1, int(round(font.getlength(TEST_CHAR))))
    else:
        bbox = font.getbbox(TEST_CHAR)
        base_width = max(1, bbox[2] - bbox[0])

    glyph_img = Image.new("L", (base_width, base_height), color=0)
    draw = ImageDraw.Draw(glyph_img)

    baseline_y = ascent
    try:
        draw.text((0, baseline_y), TEST_CHAR, font=font, fill=255, anchor="ls")
    except TypeError:
        draw.text((0, baseline_y), TEST_CHAR, font=font, fill=255)

    pixels = (np.array(glyph_img) > THRESHOLD).astype(np.uint8)

    print("Font path:", FONT_PATH)
    print("Font size (px):", font.size)
    print("Char:", TEST_CHAR)
    print("Base size (width x height):", f"{base_width} x {base_height}")
    # print("Array:")
    # print(pixels)
    print("\nBitmap preview:")
    for row in pixels:
        line = "".join("#" if v else " " for v in row)
        print(line)


if __name__ == "__main__":
    main()
