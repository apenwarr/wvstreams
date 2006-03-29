#include "wvargs.h"

int main(int argc, char *argv[])
{
    WvArgs args;
    WvStringList remaining_args;

    int int_option;

    args.add_option('i', "iopt", "integer option", "INT", int_option);
    args.add_required_arg("FILE");
    args.add_optional_arg("FILE", true);
    
    if (!args.process(argc, argv, &remaining_args))
        return 1;

    fprintf(stderr, "Printing a brief usage message\n");
    args.print_usage(argc, argv);

    fprintf(stderr, "Printing help message\n");
    args.print_help(argc, argv);

    return 0;
}
