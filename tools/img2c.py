import sys
from PIL import Image, ImageOps

def convert(src, name, w=160, h=120, flip=False):
    im = ImageOps.fit(Image.open(src).convert('RGB'), (w, h))
    if flip:
        im = ImageOps.mirror(im)
    data = []
    for r, g, b in im.getdata():
        v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
        data += [v & 0xFF, v >> 8]
    N = name.upper()
    out = [f'#define {N}_WIDTH ({w})', f'#define {N}_HEIGHT ({h})',
           f'#define {N}_BYTES_PER_PIXEL (2)',
           f'#define {N}_PIXEL_DATA ((unsigned char*) {N}_pixel_data)',
           f'static const unsigned char {N}_pixel_data[{w} * {h} * 2] = {{']
    for i in range(0, len(data), 16):
        out.append('  ' + ','.join(f'0x{x:02X}' for x in data[i:i + 16]) + ',')
    out.append('};')
    open(f'{name}.c', 'w').write('\n'.join(out) + '\n')

if __name__ == '__main__':
    convert(sys.argv[1], sys.argv[2], flip='--flip' in sys.argv)
