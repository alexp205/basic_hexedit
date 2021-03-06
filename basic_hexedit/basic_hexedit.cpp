#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdio.h>

using namespace std;

const int OFFSET_PER_LINE = 10; // number of (hex) characters (not including last space and col space)
const int HEX_PER_LINE = 48; // number of hex values in form of "xx " (not including col space)
const int ASCII_PER_LINE = 16; // number of ascii characters
int lines_per_window = 29;

int main(int argc, char *argv[])
{
    // setup
    string option_str = "-r";
    string filepath;
    int mode = 1; // 1 = read, 2 = write
    int offset = 0;

    // process args
    if (2 != argc) {
        if (3 != argc) {
            if (4 != argc) {
                printf("usage: basic_hexedit [-r/w] <filepath> [num_lines]\n");
                exit(1);
            }
            option_str = argv[1];
            filepath = argv[2];
            lines_per_window = stoi(argv[3]);
            if (lines_per_window <= 0) {
                printf("[ERROR] number of lines must be greater than 0\n");
                exit(1);
            }
        }
        option_str = argv[1];
        filepath = argv[2];
    }
    else {
        filepath = argv[1];
    }

    if (option_str.compare("-w") == 0) {
        mode = 2;
    } else if (option_str.compare("-r") == 0) {
        mode = 1;
    } else {
        printf("usage: basic_hexedit [-r/w] <filepath> [num_lines]\n");
        exit(1);
    }

    if (1 == mode) {
        // read in file
        ifstream f;
        f.open(filepath, fstream::binary);
        if (!f.is_open()) {
            printf("[ERROR] error opening file\n");
            exit(1);
        }
        bool quit = false;
        bool at_end = false;

        // read and print lines
        string hex_str;
        string ascii_str;
        int idx = 0;
        int start_offset = 0;
        int end_offset = 0;

        char c;
        while (!quit) {
            at_end = false;
            while (f.get(c) && !quit) {
                if (lines_per_window == end_offset - start_offset) {
                    // wait for input
                    char input = '.';
                    while ('j' != input && 'k' != input && 'q' != input) {
                        cin >> input;
                        if ('q' == input) {
                            quit = true;
                        } else if ('j' == input) {
                            start_offset += ASCII_PER_LINE;
                            offset = start_offset / 16 + start_offset % 16;
                        } else if ('k' == input) {
                            if (0 == start_offset) {
                                input = '.';
                            } else {
                                start_offset -= ASCII_PER_LINE;
                                offset = start_offset / 16 + start_offset % 16;
                            }
                        }
                    }

                    // clear output
                    for (int i = 0; i < lines_per_window; i++) {
                        printf("\n");
                    }

                    // reset file offset
                    f.seekg(start_offset);
                    offset = start_offset / 16 + start_offset % 16;
                    end_offset = start_offset;
                    idx = start_offset;
                } else {
                    // continue reading
                    if (isprint(c)) {
                        string s(1, c);
                        ascii_str.append(s);
                    } else {
                        string s(1, '.');
                        ascii_str.append(s);
                    }
                    stringstream sstream_hex;
                    sstream_hex << setfill('0') << setw(2) << hex << (int)c << " ";
                    hex_str.append(sstream_hex.str());

                    if (15 == idx % ASCII_PER_LINE) {
                        stringstream sstream;
                        sstream << setfill('0') << setw(OFFSET_PER_LINE) << hex << offset++;
                        string offset_str = sstream.str();
                        cout << offset_str << "  " << hex_str << " " << ascii_str << endl;

                        hex_str = "";
                        ascii_str = "";
                        end_offset++;
                    }

                    idx++;
                }
            }
            at_end = true;

            if (!quit) {
                if (0 != idx % ASCII_PER_LINE) {
                    stringstream sstream;
                    sstream << setfill('0') << setw(OFFSET_PER_LINE) << hex << offset++;
                    string offset_str = sstream.str();
                    cout << offset_str << "  " << left << setfill(' ') << setw(HEX_PER_LINE) << hex_str << " " << ascii_str << endl;
                }
                // wait for input
                char input = '.';
                while ('j' != input && 'k' != input && 'q' != input) {
                    cin >> input;
                    if ('q' == input) {
                        quit = true;
                    } else if ('j' == input) {
                        if (at_end) {
                            input = '.';
                        } else {
                            start_offset += ASCII_PER_LINE;
                            offset = start_offset / 16 + start_offset % 16;
                        }
                    } else if ('k' == input) {
                        if (0 == start_offset) {
                            input = '.';
                        } else {
                            start_offset -= ASCII_PER_LINE;
                            offset = start_offset / 16 + start_offset % 16;
                        }
                    }
                }

                // clear output
                for (int i = 0; i < lines_per_window; i++) {
                    printf("\n");
                }

                // reset file offset
                f.seekg(start_offset);
                offset = start_offset / 16 + start_offset % 16;
                end_offset = start_offset;
                idx = start_offset;
            }
        }

        // clean up
        f.close();
    } else {
        // read in file
        fstream f;
        f.open(filepath, fstream::in | fstream::out | fstream::binary);
        if (!f.is_open()) {
            printf("[ERROR] error opening file\n");
            exit(1);
        }
        bool quit = false;
        bool at_end = false;
        vector<string> disp_buff(lines_per_window);
        vector<int> select_idx{ 0, 0 }; // (x: 0 - 15, y: 0 - lines_per_window)

        // read and print lines
        string hex_str;
        string ascii_str;
        int idx = 0;
        int start_offset = 0;
        int end_offset = 0;

        char c;
        while (!quit) {
            at_end = false;
            while (f.get(c) && !quit) {
                if (lines_per_window == end_offset - start_offset) {
                    // display current selected
                    string str = disp_buff[select_idx[1]];
                    string hex_str = str.substr(OFFSET_PER_LINE + 1 + (3 * select_idx[0]) + 1, 3);
                    string ascii_str = str.substr(OFFSET_PER_LINE + 1 + HEX_PER_LINE + 1 + select_idx[0] + 1, 1);
                    cout << "\r" << hex_str << ascii_str;

                    // wait for input
                    char input = '.';
                    while ('n' != input && 'm' != input && 'q' != input && 'i' != input) {
                        cin >> input;
                        if ('q' == input) {
                            quit = true;
                        } else if ('n' == input) {
                            start_offset += ASCII_PER_LINE;
                            offset = start_offset / 16 + start_offset % 16;
                        } else if ('m' == input) {
                            if (0 == start_offset) {
                                input = '.';
                            } else {
                                start_offset -= ASCII_PER_LINE;
                                offset = start_offset / 16 + start_offset % 16;
                            }
                        } else if ('h' == input) {
                            // cursor left
                            if (select_idx[0] > 0) select_idx[0]--;
                        } else if ('j' == input) {
                            // cursor down
                            if (select_idx[1] == end_offset - start_offset - 1) {
                                select_idx[1]++;
                                select_idx[0] = 0;
                            }
                            if (select_idx[1] < end_offset - start_offset) select_idx[1]++;
                        } else if ('k' == input) {
                            // cursor up
                            if (select_idx[1] > 0) select_idx[1]--;
                        } else if ('l' == input) {
                            // cursor right
                            if ((select_idx[1] == end_offset && select_idx[0] < idx % ASCII_PER_LINE) ||
                                (select_idx[1] != end_offset && select_idx[0] < 15)) select_idx[0]++;
                        } else if ('i' == input) {
                            // wait for input
                            char replace = '.';
                            cin >> replace;
                            string s(1, replace);
                            
                            // write to file
                            int f_offset = start_offset + (16 * select_idx[1]) + select_idx[0];
                            f.close();
                            f.open(filepath, fstream::in | fstream::out | fstream::binary);
                            f.seekp(f_offset);
                            f.write(&replace, sizeof(replace));
                            f.close();
                            f.open(filepath, fstream::in | fstream::out | fstream::binary);
                        }

                        if ('h' == input || 'j' == input || 'k' == input || 'l' == input) {
                            string str = disp_buff[select_idx[1]];
                            string hex_str = str.substr(OFFSET_PER_LINE + 1 + (3 * select_idx[0]) + 1, 3);
                            string ascii_str = str.substr(OFFSET_PER_LINE + 1 + HEX_PER_LINE + 1 + select_idx[0] + 1, 1);
                            cout << "\r" << hex_str << ascii_str;
                        }
                    }

                    // clear output
                    for (int i = 0; i < lines_per_window; i++) {
                        printf("\n");
                    }

                    // reset file offset
                    f.seekg(start_offset);
                    offset = start_offset / 16 + start_offset % 16;
                    end_offset = start_offset;
                    idx = start_offset;
                } else {
                    // continue reading
                    if (isprint(c)) {
                        string s(1, c);
                        ascii_str.append(s);
                    } else {
                        string s(1, '.');
                        ascii_str.append(s);
                    }
                    stringstream sstream_hex;
                    sstream_hex << setfill('0') << setw(2) << hex << (int)c << " ";
                    hex_str.append(sstream_hex.str());

                    if (15 == idx % ASCII_PER_LINE) {
                        stringstream sstream;
                        sstream << setfill('0') << setw(OFFSET_PER_LINE) << hex << offset++;
                        string offset_str = sstream.str();
                        stringstream outstream;
                        outstream << offset_str << "  " << hex_str << " " << ascii_str << endl;
                        disp_buff[end_offset - start_offset] = outstream.str();
                        cout << outstream.str();

                        hex_str = "";
                        ascii_str = "";
                        end_offset++;
                    }

                    idx++;
                }
            }
            at_end = true;

            if (!quit) {
                if (0 != idx % ASCII_PER_LINE) {
                    stringstream sstream;
                    sstream << setfill('0') << setw(OFFSET_PER_LINE) << hex << offset++;
                    string offset_str = sstream.str();
                    stringstream outstream;
                    outstream << offset_str << "  " << left << setfill(' ') << setw(HEX_PER_LINE) << hex_str << " " << ascii_str << endl;
                    disp_buff[end_offset - start_offset] = outstream.str();
                    cout << outstream.str();

                    hex_str = "";
                    ascii_str = "";
                }
                // wait for input
                string str = disp_buff[select_idx[1]];
                string hex_str = str.substr(OFFSET_PER_LINE + 1 + (3 * select_idx[0]) + 1, 3);
                string ascii_str = str.substr(OFFSET_PER_LINE + 1 + HEX_PER_LINE + 1 + select_idx[0] + 1, 1);
                cout << "\r" << hex_str << ascii_str;
                char input = '.';
                while ('n' != input && 'm' != input && 'q' != input && 'i' != input) {
                    cin >> input;
                    if ('q' == input) {
                        quit = true;
                    } else if ('n' == input) {
                        if (at_end) {
                            input = '.';
                        } else {
                            start_offset += ASCII_PER_LINE;
                            offset = start_offset / 16 + start_offset % 16;
                        }
                    } else if ('m' == input) {
                        if (0 == start_offset) {
                            input = '.';
                        } else {
                            start_offset -= ASCII_PER_LINE;
                            offset = start_offset / 16 + start_offset % 16;
                        }
                    } else if ('h' == input) {
                        // cursor left
                        if (select_idx[0] > 0) select_idx[0]--;
                    } else if ('j' == input) {
                        // cursor down
                        if (select_idx[1] == end_offset - start_offset - 1) {
                            select_idx[1]++;
                            select_idx[0] = 0;
                        }
                        if (select_idx[1] < end_offset - start_offset) select_idx[1]++;
                    } else if ('k' == input) {
                        // cursor up
                        if (select_idx[1] > 0) select_idx[1]--;
                    } else if ('l' == input) {
                        // cursor right
                        if ((select_idx[1] == end_offset && select_idx[0] < idx % ASCII_PER_LINE) || 
                            (select_idx[1] != end_offset && select_idx[0] < 15)) select_idx[0]++;
                    } else if ('i' == input) {
                        // wait for input
                        char replace = '.';
                        cin >> replace;

                        // write to file
                        int f_offset = start_offset + (16 * select_idx[1]) + select_idx[0];
                        f.close();
                        f.open(filepath, fstream::in | fstream::out | fstream::binary);
                        f.seekp(f_offset);
                        f.write(&replace, sizeof(replace));
                        f.close();
                        f.open(filepath, fstream::in | fstream::out | fstream::binary);
                    }

                    if ('h' == input || 'j' == input || 'k' == input || 'l' == input) {
                        string str = disp_buff[select_idx[1]];
                        string hex_str = str.substr(OFFSET_PER_LINE + 1 + (3 * select_idx[0]) + 1, 3);
                        string ascii_str = str.substr(OFFSET_PER_LINE + 1 + HEX_PER_LINE + 1 + select_idx[0] + 1, 1);
                        cout << "\r" << hex_str << ascii_str;
                    }
                }

                // clear output
                for (int i = 0; i < lines_per_window; i++) {
                    printf("\n");
                }

                // reset file offset
                f.seekg(start_offset);
                offset = start_offset / 16 + start_offset % 16;
                end_offset = start_offset;
                idx = start_offset;
            }
        }

        // clean up
        f.close();
    }

    return 0;
}