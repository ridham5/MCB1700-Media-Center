import sys
import numpy as np
from PIL import Image

W, H = 320, 240


def frames(path, limit=None):
    data = np.fromfile(path, dtype='<u2')
    n = data.size // (W * H)
    data = data[: n * W * H].reshape(n, H, W)
    if limit:
        data = data[:limit]
    for f in data:
        r = ((f >> 11) & 31) * 255 // 31
        g = ((f >> 5) & 63) * 255 // 63
        b = (f & 31) * 255 // 31
        yield Image.fromarray(np.dstack([r, g, b]).astype(np.uint8))


def main(out, scale, specs):
    imgs = []
    for spec in specs:
        path, _, limit = spec.partition(':')
        imgs += list(frames(path, int(limit) if limit else None))
    imgs = [i.resize((W * scale, H * scale), Image.NEAREST).quantize(colors=128, dither=Image.Dither.NONE) for i in imgs]
    imgs[0].save(out, save_all=True, append_images=imgs[1:], duration=100, loop=0, optimize=True)


if __name__ == '__main__':
    main(sys.argv[1], int(sys.argv[2]), sys.argv[3:])
