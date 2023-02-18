#include "Logger.h"

int main() {
    for ( int i = 0; i < 50; ++i ) {
        switch ( i % 4 ) {
        case 0:
            LOG_BASE( INFO, "int: {0:d};  hex: {0:x};  oct: {0:o}", i )
            break;
        case 1:
            LOG_BASE( WARN, "int: {0:d};  hex: {0:x};  oct: {0:o}", i )
            break;
        case 2:
            LOG_BASE( DEBUG, "int: {0:d};  hex: {0:x};  oct: {0:o}", i )
            break;
        case 3:
            LOG_BASE( ERROR, "int: {0:d};  hex: {0:x};  oct: {0:o}", i )
            break;
        }
    }

    return 0;
}
