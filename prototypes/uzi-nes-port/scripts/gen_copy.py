#! /usr/bin/env python

def main():
    pages = [ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' ]

    print('\t; copy data from bank 3 to ram')
    print('\tldx #$00')
    print('copy_to_ram:')

    for page in pages:
        print('\tlda $8{}00, x'.format(page))
        print('\tsta $6{}00, x'.format(page))
    for page in pages:
        print('\tlda $9{}00, x'.format(page))
        print('\tsta $7{}00, x'.format(page))

    print('\tinx')

if __name__ == '__main__':
    main()
