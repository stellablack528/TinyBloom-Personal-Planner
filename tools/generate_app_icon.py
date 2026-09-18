from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "resources" / "icons" / "app-icon.ico"
SCALE = 4
SIZE = 256


def scaled(points):
    return tuple(int(value * SCALE) for value in points)


canvas = Image.new("RGBA", (SIZE * SCALE, SIZE * SCALE), (0, 0, 0, 0))
draw = ImageDraw.Draw(canvas)
draw.rounded_rectangle(scaled((8, 8, 248, 248)), radius=62 * SCALE, fill="#DDF3E5", outline="#B9DFC7", width=8 * SCALE)
draw.line(scaled((126, 176, 126, 91)), fill="#3D8B5D", width=14 * SCALE)
draw.polygon(
    [scaled((88, 181)), scaled((166, 181)), scaled((177, 218)), scaled((77, 218))],
    fill="#6A4535",
)
draw.ellipse(scaled((88, 169, 166, 194)), fill="#8B5A43")

left_leaf = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
left_draw = ImageDraw.Draw(left_leaf)
left_draw.ellipse(scaled((61, 64, 134, 116)), fill="#78D62C")
left_leaf = left_leaf.rotate(28, resample=Image.Resampling.BICUBIC, center=scaled((126, 110)))
canvas.alpha_composite(left_leaf)

right_leaf = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
right_draw = ImageDraw.Draw(right_leaf)
right_draw.ellipse(scaled((122, 84, 198, 138)), fill="#5DBE2E")
right_leaf = right_leaf.rotate(-28, resample=Image.Resampling.BICUBIC, center=scaled((130, 132)))
canvas.alpha_composite(right_leaf)

canvas = canvas.resize((SIZE, SIZE), Image.Resampling.LANCZOS)
OUTPUT.parent.mkdir(parents=True, exist_ok=True)
canvas.save(OUTPUT, format="ICO", sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
print(OUTPUT)
