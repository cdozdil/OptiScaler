import sys
import os

def convert_shader_to_header(input_file_path, output_file_path, array_name):
    # Read the binary file
    try:
        with open(input_file_path, 'rb') as input_file:
            byte_array = input_file.read()
    except IOError as e:
        print(f"Failed to open the input file: {input_file_path}")
        print(e)
        return

    # Open the output file
    try:
        with open(output_file_path, 'w') as output_file:
            output_file.write("#pragma once\n\n")
            output_file.write(f"const unsigned char {array_name}[] = {{\n    ")

            for i, byte in enumerate(byte_array):
                output_file.write(f"0x{byte:02x}")
                if i < len(byte_array) - 1:
                    output_file.write(", ")
                if (i + 1) % 12 == 0:
                    output_file.write("\n    ")

            output_file.write("\n};\n")

        print(f"Shader converted to header file successfully: {output_file_path}")
    except IOError as e:
        print(f"Failed to open the output file: {output_file_path}")
        print(e)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(f"Usage: {os.path.basename(__file__)} <input_shader_file> <output_header_file> <array_name>")
        sys.exit(1)

    input_shader_file = sys.argv[1]
    output_header_file = sys.argv[2]
    array_name = sys.argv[3]

    convert_shader_to_header(input_shader_file, output_header_file, array_name)
