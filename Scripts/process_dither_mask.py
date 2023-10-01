from optparse import OptionParser
import sys

from PIL import Image


def image_data(path):
    img = Image.open(path)
    if img.mode == 'P':
        img = img.convert('RGBA')
    has_alpha = img.mode == 'RGBA'
    is_monochrome = img.mode == 'L'
    is_monochrome_alpha = img.mode == 'LA'
    width, height = img.size

    if has_alpha:
        red, _, _, alpha = img.split()
        a = alpha.load()
        r = red.load()
    elif is_monochrome:
        a = None
        r = img.load()
    elif is_monochrome_alpha:
        red, alpha = img.split()
        a = alpha.load()
        r = red.load()
    else:
        red, _, _ = img.split()
        a = None
        r = red.load()
    return (width, height, r, a)


def process_dither_mask(input_image_path, output_image_path):
    width, height, values, _ = image_data(input_image_path)

    output_image = Image.new(
        mode=('L'), size=(width, height))
    output_pixels = output_image.load()
    for y in range(0, height):
        for x in range(0, width):
            output_pixels[x, y] = max(values[x, y], 1)
    output_image.save(output_image_path)


def main():
    usage = f'usage: {sys.argv[0]} <input_image> <output_image>'
    parser = OptionParser(usage=usage)

    (options, args) = parser.parse_args()
    if len(args) < 2:
        parser.print_help()
        return -1

    process_dither_mask(args[0], args[1])

    return 0


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(e)
