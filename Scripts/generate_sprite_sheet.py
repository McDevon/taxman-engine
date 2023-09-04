from optparse import OptionParser
import pathlib
import math
import sys
import os

from PIL import Image


def list_png_files(directory):
    ong_files = []
    for file in os.listdir(directory):
        if pathlib.Path(file).suffix == '.png':
            ong_files.append(file)
    return ong_files


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


def image_info(path):
    width, height, values, alphas = image_data(path)

    out_has_alpha = False
    out_has_shades = False

    for y in range(0, height):
        for x in range(0, width):
            value = values[x, y]
            full_transparent = False
            if alphas is not None:
                alpha_value = alphas[x, y]
                if alpha_value < 255:
                    out_has_alpha = True
                    if alpha_value == 0:
                        full_transparent = True
            if 0 < value < 255 and not full_transparent:
                out_has_shades = True

    return (width, height, out_has_alpha, out_has_shades)


def generate_sprite_sheet(directory, output_path):
    print('generate  ' + directory)

    output_name = os.path.basename(directory)

    files = sorted(list_png_files(directory))
    use_alpha = False
    use_shades = False
    full_size = 0
    data = {}
    file_names = []
    for file in files:
        width, height, alpha, shades = image_info(
            os.path.join(directory, file)
        )
        print(f'{str(file)} width {str(width)} height {str(height)} alpha {str(alpha)} shades {str(shades)}')
        data[file] = {
            'width': width,
            'height': height,
            'orig_w': width,
            'orig_h': height,
            'offset_x': 0,
            'offset_y': 0,
        }
        file_names.append(file)
        if alpha:
            use_alpha = True
        if shades:
            use_shades = True
        full_size += width * height

    edge = math.ceil(math.sqrt(full_size))
    print(f'full size {full_size} edge {edge}')
    output_image = Image.new(
        mode=('LA' if use_alpha else 'L'), size=(edge, edge))
    output_pixels = output_image.load()
    image_start = 0
    for file in files:
        width, height, values, alphas = image_data(
            os.path.join(directory, file)
        )
        for y in range(0, height):
            for x in range(0, width):
                intput_index = x + y * width
                output_index = image_start + intput_index
                if use_alpha:
                    alpha_value = alphas[x, y] if alphas is not None else 255
                    output_pixels[output_index % edge,
                                  int(math.floor(output_index / edge))] = (
                                      values[x, y],
                                      alpha_value
                    )
                else:
                    output_pixels[output_index % edge,
                                  int(math.floor(output_index / edge))] = values[x, y]
        data[file]['start'] = image_start
        image_start += width * height
    image_name = output_name + '.png'
    output_image.save(os.path.join(output_path, image_name))
    write_sprite_sheet_data(file_names, data, image_name, os.path.join(output_path, output_name + '.txt'))


def write_sprite_sheet_data(file_names, data, atlas_image_name, output_path):
    file = open(output_path, 'w', encoding='UTF-8')
    file.write(atlas_image_name + '\n')

    for file_name in file_names:
        file_dict = data[file_name]
        file.write(file_name + '\n')
        file.write(f"  start: {str(file_dict['start'])}\n")
        file.write(f"  size: {str(file_dict['width'])} {str(file_dict['height'])}\n")
        file.write(f"  orig: {str(file_dict['orig_w'])} {str(file_dict['orig_h'])}\n")
        file.write(f"  offset: {str(file_dict['offset_x'])} {str(file_dict['offset_y'])}\n")

    file.close()


def main():
    usage = f'usage: {sys.argv[0]} [options] <sprite_directory>'
    parser = OptionParser(usage=usage)
    parser.add_option('-o', '--output', default='.',
                      help='output path for the sprite sheet, default = .')

    (options, args) = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        return -1

    sprite_directory = os.path.normpath(args[0])
    print('Generate sprite sheet from ' + sprite_directory)

    generate_sprite_sheet(sprite_directory, options.output)

    return 0


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(e)
