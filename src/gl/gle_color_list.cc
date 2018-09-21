// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


#include "gle_color_list.h"
#include "exceptions.h"
#include <iomanip>

/// list of unnamed colors
gle_color nice_colors[] = {
    0xFFFFFFFF,
    0xFFA500FF,
    0x00BB00FF,
    0x3366FFFF,
    0x7D7D7DFF,
    0x3366FFFF,
    0xFF4500FF,
    0xFFFF00FF,
    0xAFD7FFFF,
    0xAF7559FF,
    0x00FF00FF,
    0x2E8B57FF,
    0x98FFB3FF,
    0xFF0065FF,
    0xFF00FFFF,
    0xFFA500FF,
    0xFF6575FF,
    0xFFABBBFF,
    0xA020F0FF,
    0xFF0000FF,
    0x00FA6DFF,
    0xEE82EEFF,
    0xAAAAAAFF,
    0x3A90FFFF,
    0x00FFFFFF,
    0xF6F675FF,
    0xFC9CAAFF};



/// List of named colors
gle::named_color common_colors[] = {
    {"white",         0xFFFFFFFF},
    {"aqua",          0x33FFCCFF},
    {"aquamarine",    0x66DDAAFF},
    {"beige",         0x908470FF},
    {"black",         0x000000FF},
    {"blue",          0x0000FFFF},
    {"blue_tint",     0xAFD7FFFF},
    {"brick_red",     0x990000FF},
    {"brown",         0xAF7559FF},
    {"burgundy",      0x800000FF},
    {"coral",         0xF76541FF},
    {"crimson",       0xCC0033FF},
    {"cyan",          0x00FFFFFF},
    {"dark_blue",     0x000080FF},
    {"dark_gray",     0x404040FF},
    {"dark_green",    0x008000FF},
    {"dark_purple",   0x800080FF},
    {"dark_red",      0x800000FF},
    {"dark_yellow",   0x808000FF},
    {"dull_blue",     0x006699FF},
    {"firebrick",     0x800517FF},
    {"forest_green",  0x808000FF},
    {"fuscia",        0xFF00FFFF},
    {"flesh",         0xF4D3B4FF},
    {"gold",          0xD4A017FF},
    {"grass_green",   0x408080FF},
    {"gray",          0x707070FF},
    {"green",         0x00FF00FF},
    {"green_blue",    0x2E8B57FF},
    {"green_tint",    0x98FFB3FF},
    {"hot_pink",      0xFF66CCFF},
    {"khaki",         0xADA96EFF},
    {"lavender",      0x9966CCFF},
    {"light_blue",    0x9999FFFF},
    {"light_gray",    0xA0A0A0FF},
    {"light_green",   0x99FF99FF},
    {"light_purple",  0xFF0080FF},
    {"light_red",     0xFF9999FF},
    {"magenta",       0xFF00FFFF},
    {"maroon",        0x810541FF},
    {"mauve",         0xCC3366FF},
    {"navy",          0x000080FF},
    {"olive",         0x808000FF},
    {"orange",        0xFFA500FF},
    {"pastel_green",  0x00FF00FF},
    {"pink",          0xE7A1B0FF},
    {"pink_tint",     0xFFABBBFF},
    {"plum",          0xB93B8FFF},
    {"pumpkin",       0xFF9933FF},
    {"purple",        0x8E35EFFF},
    {"red",           0xFF0000FF},
    {"red_orange",    0xFF4500FF},
    {"royal_blue",    0x2B60DEFF},
    {"salmon",        0xFA8072FF},
    {"sea_green",     0x00FA6DFF},
    {"sienna",        0x8A4117FF},
    {"sky_blue",      0x3A90FFFF},
    {"steel_blue",    0x4863A0FF},
    {"turquoise",     0x00FFFFFF},
    {"violet",        0x8D38C9FF},
    {"yellow",        0xFFFF00FF},
    {"yellow_tint",   0xF6F675FF}
};


/// standard named HTML colors
gle::named_color html_colors[] = {
    {"AliceBlue",            0xF0F8FFFF},
    {"AntiqueWhite",         0xFAEBD7FF},
    {"Aqua",                 0x00FFFFFF},
    {"Aquamarine",           0x7FFFD4FF},
    {"Azure",                0xF0FFFFFF},
    {"Beige",                0xF5F5DCFF},
    {"Bisque",               0xFFE4C4FF},
    {"Black",                0x000000FF},
    {"BlanchedAlmond",       0xFFEBCDFF},
    {"Blue",                 0x0000FFFF},
    {"BlueViolet",           0x8A2BE2FF},
    {"Brown",                0xA52A2AFF},
    {"BurlyWood",            0xDEB887FF},
    {"CadetBlue",            0x5F9EA0FF},
    {"Chartreuse",           0x7FFF00FF},
    {"Chocolate",            0xD2691EFF},
    {"Coral",                0xFF7F50FF},
    {"CornflowerBlue",       0x6495EDFF},
    {"Cornsilk",             0xFFF8DCFF},
    {"Crimson",              0xDC143CFF},
    {"Cyan",                 0x00FFFFFF},
    {"DarkBlue",             0x00008BFF},
    {"DarkCyan",             0x008B8BFF},
    {"DarkGoldenRod",        0xB8860BFF},
    {"DarkGray",             0xA9A9A9FF},
    {"DarkGrey",             0xA9A9A9FF},
    {"DarkGreen",            0x006400FF},
    {"DarkKhaki",            0xBDB76BFF},
    {"DarkMagenta",          0x8B008BFF},
    {"DarkOliveGreen",       0x556B2FFF},
    {"Darkorange",           0xFF8C00FF},
    {"DarkOrchid",           0x9932CCFF},
    {"DarkRed",              0x8B0000FF},
    {"DarkSalmon",           0xE9967AFF},
    {"DarkSeaGreen",         0x8FBC8FFF},
    {"DarkSlateBlue",        0x483D8BFF},
    {"DarkSlateGray",        0x2F4F4FFF},
    {"DarkSlateGrey",        0x2F4F4FFF},
    {"DarkTurquoise",        0x00CED1FF},
    {"DarkViolet",           0x9400D3FF},
    {"DeepPink",             0xFF1493FF},
    {"DeepSkyBlue",          0x00BFFFFF},
    {"DimGray",              0x696969FF},
    {"DimGrey",              0x696969FF},
    {"DodgerBlue",           0x1E90FFFF},
    {"FireBrick",            0xB22222FF},
    {"FloralWhite",          0xFFFAF0FF},
    {"ForestGreen",          0x228B22FF},
    {"Fuchsia",              0xFF00FFFF},
    {"Gainsboro",            0xDCDCDCFF},
    {"GhostWhite",           0xF8F8FFFF},
    {"Gold",                 0xFFD700FF},
    {"GoldenRod",            0xDAA520FF},
    {"Gray",                 0x808080FF},
    {"Grey",                 0x808080FF},
    {"Green",                0x008000FF},
    {"GreenYellow",          0xADFF2FFF},
    {"HoneyDew",             0xF0FFF0FF},
    {"HotPink",              0xFF69B4FF},
    {"IndianRed",            0xCD5C5CFF},
    {"Indigo",               0x4B0082FF},
    {"Ivory",                0xFFFFF0FF},
    {"Khaki",                0xF0E68CFF},
    {"Lavender",             0xE6E6FAFF},
    {"LavenderBlush",        0xFFF0F5FF},
    {"LawnGreen",            0x7CFC00FF},
    {"LemonChiffon",         0xFFFACDFF},
    {"LightBlue",            0xADD8E6FF},
    {"LightCoral",           0xF08080FF},
    {"LightCyan",            0xE0FFFFFF},
    {"LightGoldenRodYellow", 0xFAFAD2FF},
    {"LightGray",            0xD3D3D3FF},
    {"LightGrey",            0xD3D3D3FF},
    {"LightGreen",           0x90EE90FF},
    {"LightPink",            0xFFB6C1FF},
    {"LightSalmon",          0xFFA07AFF},
    {"LightSeaGreen",        0x20B2AAFF},
    {"LightSkyBlue",         0x87CEFAFF},
    {"LightSlateGray",       0x778899FF},
    {"LightSlateGrey",       0x778899FF},
    {"LightSteelBlue",       0xB0C4DEFF},
    {"LightYellow",          0xFFFFE0FF},
    {"Lime",                 0x00FF00FF},
    {"LimeGreen",            0x32CD32FF},
    {"Linen",                0xFAF0E6FF},
    {"Magenta",              0xFF00FFFF},
    {"Maroon",               0x800000FF},
    {"MediumAquaMarine",     0x66CDAAFF},
    {"MediumBlue",           0x0000CDFF},
    {"MediumOrchid",         0xBA55D3FF},
    {"MediumPurple",         0x9370D8FF},
    {"MediumSeaGreen",       0x3CB371FF},
    {"MediumSlateBlue",      0x7B68EEFF},
    {"MediumSpringGreen",    0x00FA9AFF},
    {"MediumTurquoise",      0x48D1CCFF},
    {"MediumVioletRed",      0xC71585FF},
    {"MidnightBlue",         0x191970FF},
    {"MintCream",            0xF5FFFAFF},
    {"MistyRose",            0xFFE4E1FF},
    {"Moccasin",             0xFFE4B5FF},
    {"NavajoWhite",          0xFFDEADFF},
    {"Navy",                 0x000080FF},
    {"OldLace",              0xFDF5E6FF},
    {"Olive",                0x808000FF},
    {"OliveDrab",            0x6B8E23FF},
    {"Orange",               0xFFA500FF},
    {"OrangeRed",            0xFF4500FF},
    {"Orchid",               0xDA70D6FF},
    {"PaleGoldenRod",        0xEEE8AAFF},
    {"PaleGreen",            0x98FB98FF},
    {"PaleTurquoise",        0xAFEEEEFF},
    {"PaleVioletRed",        0xD87093FF},
    {"PapayaWhip",           0xFFEFD5FF},
    {"PeachPuff",            0xFFDAB9FF},
    {"Peru",                 0xCD853FFF},
    {"Pink",                 0xFFC0CBFF},
    {"Plum",                 0xDDA0DDFF},
    {"PowderBlue",           0xB0E0E6FF},
    {"Purple",               0x800080FF},
    {"Red",                  0xFF0000FF},
    {"RosyBrown",            0xBC8F8FFF},
    {"RoyalBlue",            0x4169E1FF},
    {"SaddleBrown",          0x8B4513FF},
    {"Salmon",               0xFA8072FF},
    {"SandyBrown",           0xF4A460FF},
    {"SeaGreen",             0x2E8B57FF},
    {"SeaShell",             0xFFF5EEFF},
    {"Sienna",               0xA0522DFF},
    {"Silver",               0xC0C0C0FF},
    {"SkyBlue",              0x87CEEBFF},
    {"SlateBlue",            0x6A5ACDFF},
    {"SlateGray",            0x708090FF},
    {"SlateGrey",            0x708090FF},
    {"Snow",                 0xFFFAFAFF},
    {"SpringGreen",          0x00FF7FFF},
    {"SteelBlue",            0x4682B4FF},
    {"Tan",                  0xD2B48CFF},
    {"Teal",                 0x008080FF},
    {"Thistle",              0xD8BFD8FF},
    {"Tomato",               0xFF6347FF},
    {"Turquoise",            0x40E0D0FF},
    {"Violet",               0xEE82EEFF},
    {"Wheat",                0xF5DEB3FF},
    {"White",                0xFFFFFFFF},
    {"WhiteSmoke",           0xF5F5F5FF},
    {"Yellow",               0xFFFF00FF},
    {"YellowGreen",          0x9ACD32FF}
};


/// a list of ~180 colors from Crayola crayons
gle::named_color crayola_colors[] = {
    {"Almond",                0xEFDECDFF},
    {"Amethyst",              0x64609AFF},
    {"Antique Brass",         0xCD9575FF},
    {"Apricot",               0xFDD9B5FF},
    {"Aquamarine",            0x78DBE2FF},
    {"Asparagus",             0x87A96BFF},
    {"Atomic Tangerine",      0xFFA474FF},
    {"Baby Powder",           0xfefefaFF},
    {"Banana",                0xffd12aFF},
    {"Banana Mania",          0xFAE7B5FF},
    {"Beaver",                0x9F8170FF},
    {"Bittersweet",           0xFD7C6EFF},
    {"Black",                 0x000000FF},
    {"Blueberry",             0x4f86f7FF},
    {"Blizzard Blue",         0xACE5EEFF},
    {"Blue",                  0x1F75FEFF},
    {"Blue Bell",             0xA2A2D0FF},
    {"Blue Gray",             0x6699CCFF},
    {"Blue Green",            0x0D98BAFF},
    {"Blue Violet",           0x7366BDFF},
    {"Blush",                 0xDE5D83FF},
    {"Brick Red",             0xCB4154FF},
    {"Brown",                 0xB4674DFF},
    {"Bubble Gum",            0xffd3f8FF},
    {"Burnt Orange",          0xFF7F49FF},
    {"Burnt Sienna",          0xEA7E5DFF},
    {"Cadet Blue",            0xB0B7C6FF},
    {"Canary",                0xFFFF99FF},
    {"Caribbean Green",       0x1CD3A2FF},
    {"Carnation Pink",        0xFFAACCFF},
    {"Cedar Chest",           0xc95a49FF},
    {"Cerise",                0xDD4492FF},
    {"Cerulean",              0x1DACD6FF},
    {"Cherry",                0xda2647FF},
    {"Chestnut",              0xBC5D58FF},
    {"Chocolate",             0xbd8260FF},
    {"Citrine",               0x933709FF},
    {"Coconut",               0xfefefeFF},
    {"Copper",                0xDD9475FF},
    {"Cornflower",            0x9ACEEBFF},
    {"Cotton Candy",          0xFFBCD9FF},
    {"Daffodil",              0xffff31FF},
    {"Dandelion",             0xFDDB6DFF},
    {"Denim",                 0x2B6CC4FF},
    {"Desert Sand",           0xEFCDB8FF},
    {"Dirt",                  0x9b7653FF},
    {"Eggplant",              0x6E5160FF},
    {"Electric Lime",         0xCEFF1DFF},
    {"Emerald",               0x14A989FF},
    {"Eucalyptus",            0x44d7a8FF},
    {"Fern",                  0x71BC78FF},
    {"Forest Green",          0x6DAE81FF},
    {"Fresh Air",             0xa6e7ffFF},
    {"Fuchsia",               0xC364C5FF},
    {"Fuzzy Wuzzy",           0xCC6666FF},
    {"Gold",                  0xE7C697FF},
    {"Goldenrod",             0xFCD975FF},
    {"Granny Smith Apple",    0xA8E4A0FF},
    {"Grape",                 0x6f2da8FF},
    {"Gray",                  0x95918CFF},
    {"Green",                 0x1CAC78FF},
    {"Green Blue",            0x1164B4FF},
    {"Green Yellow",          0xF0E891FF},
    {"Hot Magenta",           0xFF1DCEFF},
    {"Inchworm",              0xB2EC5DFF},
    {"Indigo",                0x5D76CBFF},
    {"Jade",                  0x469A84FF},
    {"Jasper",                0xD05340FF},
    {"Jazzberry Jam",         0xCA3767FF},
    {"Jelly Bean",            0xda614eFF},
    {"Jungle Green",          0x3BB08FFF},
    {"Lapis Lazuli",          0x436CB9FF},
    {"Laser Lemon",           0xFEFE22FF},
    {"Lavender",              0xFCB4D5FF},
    {"Leather Jacket",        0x253529FF},
    {"Lemon",                 0xffff38FF},
    {"Lemon Yellow",          0xFFF44FFF},
    {"Licorice",              0x1a1110FF},
    {"Lilac",                 0xdb91efFF},
    {"Lime",                  0xb2f302FF},
    {"Lumber",                0xffe4cdFF},
    {"Macaroni and Cheese",   0xFFBD88FF},
    {"Magenta",               0xF664AFFF},
    {"Magic Mint",            0xAAF0D1FF},
    {"Mahogany",              0xCD4A4CFF},
    {"Maize",                 0xEDD19CFF},
    {"Malachite",             0x469496FF},
    {"Manatee",               0x979AAAFF},
    {"Mango Tango",           0xFF8243FF},
    {"Maroon",                0xC8385AFF},
    {"Mauvelous",             0xEF98AAFF},
    {"Melon",                 0xFDBCB4FF},
    {"Midnight Blue",         0x1A4876FF},
    {"Moonstone",             0x3AA8C1FF},
    {"Mountain Meadow",       0x30BA8FFF},
    {"Mulberry",              0xC54B8CFF},
    {"Navy Blue",             0x1974D2FF},
    {"Neon Carrot",           0xFFA343FF},
    {"New Car",               0x214fc6FF},
    {"Olive Green",           0xBAB86CFF},
    {"Onyx",                  0x353839FF},
    {"Orange",                0xFF7538FF},
    {"Orange Red",            0xFF2B2BFF},
    {"Orange Scent",          0xff8866FF},
    {"Orange Yellow",         0xF8D568FF},
    {"Orchid",                0xE6A8D7FF},
    {"Outer Space",           0x414A4CFF},
    {"Outrageous Orange",     0xFF6E4AFF},
    {"Pacific Blue",          0x1CA9C9FF},
    {"Peach",                 0xFFCFABFF},
    {"Peach Scent",           0xffd0b9FF},
    {"Peridot",               0xABAD48FF},
    {"Periwinkle",            0xC5D0E6FF},
    {"Piggy Pink",            0xFDDDE6FF},
    {"Pine",                  0x45a27dFF},
    {"Pine Green",            0x158078FF},
    {"Pink Flamingo",         0xFC74FDFF},
    {"Pink Pearl",            0xB07080FF},
    {"Pink Sherbert",         0xF78FA7FF},
    {"Plum",                  0x8E4585FF},
    {"Purple Heart",          0x7442C8FF},
    {"Purple Majesty",        0x9D81BAFF},
    {"Purple Pizzazz",        0xFE4EDAFF},
    {"Radical Red",           0xFF496CFF},
    {"Raw Sienna",            0xD68A59FF},
    {"Raw Umber",             0x714B23FF},
    {"Razzle Dazzle Rose",    0xFF48D0FF},
    {"Razzmatazz",            0xE3256BFF},
    {"Red",                   0xEE204DFF},
    {"Red Orange",            0xFF5349FF},
    {"Red Violet",            0xC0448FFF},
    {"Robin's Egg Blue",      0x1FCECBFF},
    {"Rose",                  0xff5050FF},
    {"Rose Quartz",           0xBD559CFF},
    {"Royal Purple",          0x7851A9FF},
    {"Ruby",                  0xAA4069FF},
    {"Salmon",                0xFF9BAAFF},
    {"Sapphire",              0x2D5DA1FF},
    {"Scarlet",               0xFC2847FF},
    {"Screamin' Green",       0x76FF7AFF},
    {"Sea Green",             0x9FE2BFFF},
    {"Sepia",                 0xA5694FFF},
    {"Shadow",                0x8A795DFF},
    {"Shampoo",               0xffcff1FF},
    {"Shamrock",              0x45CEA2FF},
    {"Shocking Pink",         0xFB7EFDFF},
    {"Silver",                0xCDC5C2FF},
    {"Sky Blue",              0x80DAEBFF},
    {"Smoke",                 0x738276FF},
    {"Smokey Topaz",          0x832A0DFF},
    {"Soap",                  0xcec8efFF},
    {"Spring Green",          0xECEABEFF},
    {"Strawberry",            0xfc5a8dFF},
    {"Sunglow",               0xFFCF48FF},
    {"Sunset Orange",         0xFD5E53FF},
    {"Tan",                   0xFAA76CFF},
    {"Teal Blue",             0x18A7B5FF},
    {"Thistle",               0xEBC7DFFF},
    {"Tickle Me Pink",        0xFC89ACFF},
    {"Tiger's Eye",           0xB56917FF},
    {"Timberwolf",            0xDBD7D2FF},
    {"Tropical Rain Forest",  0x17806DFF},
    {"Tulip",                 0xff878dFF},
    {"Tumbleweed",            0xDEAA88FF},
    {"Turquoise Blue",        0x77DDE7FF},
    {"Unmellow Yellow",       0xFFFF66FF},
    {"Violet (Purple)",       0x926EAEFF},
    {"Violet Blue",           0x324AB2FF},
    {"Violet Red",            0xF75394FF},
    {"Vivid Tangerine",       0xFFA089FF},
    {"Vivid Violet",          0x8F509DFF},
    {"White",                 0xFFFFFFFF},
    {"Wild Blue Yonder",      0xA2ADD0FF},
    {"Wild Strawberry",       0xFF43A4FF},
    {"Wild Watermelon",       0xFC6C85FF},
    {"Wisteria",              0xCDA4DEFF},
    {"Yellow",                0xFCE883FF},
    {"Yellow Green",          0xC5E384FF},
    {"Yellow Orange",         0xFFAE42FF}
};


gle_color gle::std_color(size_t indx)
{
    const unsigned max = sizeof(common_colors) / sizeof(named_color);
    return common_colors[ indx % max ].hex;
}


gle_color gle::std_color(const std::string& name)
{
    const unsigned max = sizeof(common_colors) / sizeof(named_color);
    
    for ( unsigned c = 0; c < max; ++c )
        if ( common_colors[c].name == name )
            return common_colors[c].hex;
    
    throw InvalidSyntax("Unknown color `"+name+"'");
    return 0xFFFFFFFF;
}


void gle::print_std_colors(std::ostream & os)
{
    os << "Known colors:" << std::endl;
    print_colors(os, common_colors, sizeof(common_colors)/sizeof(named_color));
}



gle_color gle::alt_color(unsigned indx)
{
    const unsigned max = sizeof(crayola_colors) / sizeof(named_color);
    return crayola_colors[ indx % max ].hex;
}


/**
 extract all colors from the crayola list, that have a brightness between `minb` and `maxb`
 */

int gle::select_colors(gle_color* array, unsigned asize,
                       const GLfloat minb, const GLfloat maxb)
{
    unsigned max = sizeof(crayola_colors) / sizeof(named_color);
    assert_true( minb < maxb );

    unsigned nbc = 0;
    for ( unsigned c = 0; c < max; ++c )
    {
        gle_color col = crayola_colors[c].hex;
        if ( minb <= col.brightness()  &&  col.brightness() <= maxb )
            array[nbc++] = col;
        if ( nbc == asize )
            return nbc;
    }
    
    // always returns at least one color:
    if ( nbc == 0 && asize > 0 )
    {
        array[0] = 0xFFFFFFFF;
        nbc = 1;
    }
    
    return nbc;
}


/**
 return `indx`-th color from the crayola list, that has a brightness between `minb` and `maxb`
 */
gle_color gle::bright_color(size_t indx, const GLfloat minb, const GLfloat maxb)
{
    const unsigned max = sizeof(crayola_colors) / sizeof(named_color);

    static gle_color scolor[max];
    static GLfloat sminb = 0.6, smaxb = 3;
    static unsigned scmax = select_colors(scolor, max, sminb, smaxb);

    if ( sminb != minb || smaxb != maxb )
    {
        sminb = minb;
        smaxb = maxb;
        scmax = select_colors(scolor, max, minb, maxb);
    }
    
    return scolor[indx%scmax];
}


gle_color gle::nice_color(size_t indx)
{
    const unsigned max = sizeof(nice_colors) / sizeof(gle_color);
    return nice_colors[ indx % max ];
}


void gle::print_colors(std::ostream & os, named_color list[], unsigned max)
{    
    for ( unsigned c = 0; c < max; ++c )
    {
        os << std::setw(16) << list[c].name << "  ";
        os << gle_color( list[c].hex ) << std::endl;
    }
}


