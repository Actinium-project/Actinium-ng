// Copyright (c) 2017, 2021 Pieter Wuille
// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bech32.h>
#include <util/vector.h>

#include <assert.h>

namespace bech32
{

namespace
{

typedef std::vector<uint8_t> data;

/** The Bech32 and Bech32m character set for encoding. */
const char* CHARSET = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

/** The Bech32 and Bech32m character set for decoding. */
const int8_t CHARSET_REV[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    15, -1, 10, 17, 21, 20, 26, 30,  7,  5, -1, -1, -1, -1, -1, -1,
    -1, 29, -1, 24, 13, 25,  9,  8, 23, -1, 18, 22, 31, 27, 19, -1,
     1,  0,  3, 16, 11, 28, 12, 14,  6,  4,  2, -1, -1, -1, -1, -1,
    -1, 29, -1, 24, 13, 25,  9,  8, 23, -1, 18, 22, 31, 27, 19, -1,
     1,  0,  3, 16, 11, 28, 12, 14,  6,  4,  2, -1, -1, -1, -1, -1
};

// We work with the finite field GF(1024) defined as a degree 2 extension of the base field GF(32)
// The defining polynomial of the extension is x^2 + 9x + 23
// Let (e) be a primitive element of GF(1024), that is, a generator of the field.
// Every non-zero element of the field can then be represented as (e)^k for some power k.
// The array GF1024_EXP contains all these powers of (e) - GF1024_EXP[k] = (e)^k in GF(1024).
// Conversely, GF1024_LOG contains the discrete logarithms of these powers, so
// GF1024_LOG[GF1024_EXP[k]] == k
// Each element v of GF(1024) is encoded as a 10 bit integer in the following way:
// v = v1 || v0 where v0, v1 are 5-bit integers (elements of GF(32)).
//
// The element (e) is encoded as 9 || 15. Given (v), we compute (e)*(v) by multiplying in the following way:
// v0' = 27*v1 + 15*v0
// v1' = 6*v1 + 9*v0
// e*v = v1' || v0'
//
// The following sage code can be used to reproduce both _EXP and _LOG arrays
// GF1024_LOG = [-1] + [0] * 1023
// GF1024_EXP = [1] * 1024
// v = 1
// for i in range(1, 1023):
//     v0 = v & 31
//     v1 = v >> 5
//     v0n = F.fetch_int(27)*F.fetch_int(v1) + F.fetch_int(15)*F.fetch_int(v0)
//     v1n = F.fetch_int(6)*F.fetch_int(v1) + F.fetch_int(9)*F.fetch_int(v0)
//     v = v1n.integer_representation() << 5 | v0n.integer_representation()
//     GF1024_EXP[i] = v
//     GF1024_LOG[v] = i

const int16_t GF1024_EXP[] = {
    1, 303, 635, 446, 997, 640, 121, 142, 959, 420, 350, 438, 166, 39, 543,
    335, 831, 691, 117, 632, 719, 97, 107, 374, 558, 797, 54, 150, 858, 877,
    724, 1013, 294, 23, 354, 61, 164, 633, 992, 538, 469, 659, 174, 868, 184,
    809, 766, 563, 866, 851, 257, 520, 45, 770, 535, 524, 408, 213, 436, 760,
    472, 330, 933, 799, 616, 361, 15, 391, 756, 814, 58, 608, 554, 680, 993,
    821, 942, 813, 843, 484, 193, 935, 321, 919, 572, 741, 423, 559, 562,
    589, 296, 191, 493, 685, 891, 665, 435, 60, 395, 2, 606, 511, 853, 746,
    32, 219, 284, 631, 840, 661, 837, 332, 78, 311, 670, 887, 111, 195, 505,
    190, 194, 214, 709, 380, 819, 69, 261, 957, 1018, 161, 739, 588, 7, 708,
    83, 328, 507, 736, 317, 899, 47, 348, 1000, 345, 882, 245, 367, 996, 943,
    514, 304, 90, 804, 295, 312, 793, 387, 833, 249, 921, 660, 618, 823, 496,
    722, 30, 782, 225, 892, 93, 480, 372, 112, 738, 867, 636, 890, 950, 968,
    386, 622, 642, 551, 369, 234, 846, 382, 365, 442, 592, 343, 986, 122,
    1023, 59, 847, 81, 790, 4, 437, 983, 931, 244, 64, 415, 529, 487, 944,
    35, 938, 664, 156, 583, 53, 999, 222, 390, 987, 341, 388, 389, 170, 721,
    879, 138, 522, 627, 765, 322, 230, 440, 14, 168, 143, 656, 991, 224, 595,
    550, 94, 657, 752, 667, 1005, 451, 734, 744, 638, 292, 585, 157, 872,
    590, 601, 827, 774, 930, 475, 571, 33, 500, 871, 969, 173, 21, 828, 450,
    1009, 147, 960, 705, 201, 228, 998, 497, 1021, 613, 688, 772, 508, 36,
    366, 715, 468, 956, 725, 730, 861, 425, 647, 701, 221, 759, 95, 958, 139,
    805, 8, 835, 679, 614, 449, 128, 791, 299, 974, 617, 70, 628, 57, 273,
    430, 67, 750, 405, 780, 703, 643, 776, 778, 340, 171, 1022, 276, 308,
    495, 243, 644, 460, 857, 28, 336, 286, 41, 695, 448, 431, 364, 149, 43,
    233, 63, 762, 902, 181, 240, 501, 584, 434, 275, 1008, 444, 443, 895,
    812, 612, 927, 383, 66, 961, 1006, 690, 346, 3, 881, 900, 747, 271, 672,
    162, 402, 456, 748, 971, 755, 490, 105, 808, 977, 72, 732, 182, 897, 625,
    163, 189, 947, 850, 46, 115, 403, 231, 151, 629, 278, 874, 16, 934, 110,
    492, 898, 256, 807, 598, 700, 498, 140, 481, 91, 523, 860, 134, 252, 771,
    824, 119, 38, 816, 820, 641, 342, 757, 513, 577, 990, 463, 40, 920, 955,
    17, 649, 533, 82, 103, 896, 862, 728, 259, 86, 466, 87, 253, 556, 323,
    457, 963, 432, 845, 527, 745, 849, 863, 1015, 888, 488, 567, 727, 132,
    674, 764, 109, 669, 6, 1003, 552, 246, 542, 96, 324, 781, 912, 248, 694,
    239, 980, 210, 880, 683, 144, 177, 325, 546, 491, 326, 339, 623, 941, 92,
    207, 783, 462, 263, 483, 517, 1012, 9, 620, 220, 984, 548, 512, 878, 421,
    113, 973, 280, 962, 159, 310, 945, 268, 465, 806, 889, 199, 76, 873, 865,
    34, 645, 227, 290, 418, 693, 926, 80, 569, 639, 11, 50, 291, 141, 206,
    544, 949, 185, 518, 133, 909, 135, 467, 376, 646, 914, 678, 841, 954,
    318, 242, 939, 951, 743, 1017, 976, 359, 167, 264, 100, 241, 218, 51, 12,
    758, 368, 453, 309, 192, 648, 826, 553, 473, 101, 478, 673, 397, 1001,
    118, 265, 331, 650, 356, 982, 652, 655, 510, 634, 145, 414, 830, 924,
    526, 966, 298, 737, 18, 504, 401, 697, 360, 288, 1020, 842, 203, 698,
    537, 676, 279, 581, 619, 536, 907, 876, 1019, 398, 152, 1010, 994, 68,
    42, 454, 580, 836, 99, 565, 137, 379, 503, 22, 77, 582, 282, 412, 352,
    611, 347, 300, 266, 570, 270, 911, 729, 44, 557, 108, 946, 637, 597, 461,
    630, 615, 238, 763, 681, 718, 334, 528, 200, 459, 413, 79, 24, 229, 713,
    906, 579, 384, 48, 893, 370, 923, 202, 917, 98, 794, 754, 197, 530, 662,
    52, 712, 677, 56, 62, 981, 509, 267, 789, 885, 561, 316, 684, 596, 226,
    13, 985, 779, 123, 720, 576, 753, 948, 406, 125, 315, 104, 519, 426, 502,
    313, 566, 1016, 767, 796, 281, 749, 740, 136, 84, 908, 424, 936, 198,
    355, 274, 735, 967, 5, 154, 428, 541, 785, 704, 486, 671, 600, 532, 381,
    540, 574, 187, 88, 378, 216, 621, 499, 419, 922, 485, 494, 476, 255, 114,
    188, 668, 297, 400, 918, 787, 158, 25, 458, 178, 564, 422, 768, 73, 1011,
    717, 575, 404, 547, 196, 829, 237, 394, 301, 37, 65, 176, 106, 89, 85,
    675, 979, 534, 803, 995, 363, 593, 120, 417, 452, 26, 699, 822, 223, 169,
    416, 235, 609, 773, 211, 607, 208, 302, 852, 965, 603, 357, 761, 247,
    817, 539, 250, 232, 272, 129, 568, 848, 624, 396, 710, 525, 183, 686, 10,
    285, 856, 307, 811, 160, 972, 55, 441, 289, 723, 305, 373, 351, 153, 733,
    409, 506, 975, 838, 573, 970, 988, 913, 471, 205, 337, 49, 594, 777, 549,
    815, 277, 27, 916, 333, 353, 844, 800, 146, 751, 186, 375, 769, 358, 392,
    883, 474, 788, 602, 74, 130, 329, 212, 155, 131, 102, 687, 293, 870, 742,
    726, 427, 217, 834, 904, 29, 127, 869, 407, 338, 832, 470, 482, 810, 399,
    439, 393, 604, 929, 682, 447, 714, 251, 455, 875, 319, 477, 464, 521,
    258, 377, 937, 489, 792, 172, 314, 327, 124, 20, 531, 953, 591, 886, 320,
    696, 71, 859, 578, 175, 587, 707, 663, 283, 179, 795, 989, 702, 940, 371,
    692, 689, 555, 903, 410, 651, 75, 429, 818, 362, 894, 515, 31, 545, 666,
    706, 952, 864, 269, 254, 349, 711, 802, 716, 784, 1007, 925, 801, 445,
    148, 260, 658, 385, 287, 262, 204, 126, 586, 1004, 236, 165, 854, 411,
    932, 560, 19, 215, 1002, 775, 653, 928, 901, 964, 884, 798, 839, 786,
    433, 610, 116, 855, 180, 479, 910, 1014, 599, 915, 905, 306, 516, 731,
    626, 978, 825, 344, 605, 654, 209
};
// As above, GF1024_EXP contains all elements of GF(1024) except 0
static_assert(std::size(GF1024_EXP) == 1023, "GF1024_EXP length should be 1023");

const int16_t GF1024_LOG[] = {
    -1, 0, 99, 363, 198, 726, 462, 132, 297, 495, 825, 528, 561, 693, 231,
    66, 396, 429, 594, 990, 924, 264, 627, 33, 660, 759, 792, 858, 330, 891,
    165, 957, 104, 259, 518, 208, 280, 776, 416, 13, 426, 333, 618, 339, 641,
    52, 388, 140, 666, 852, 529, 560, 678, 213, 26, 832, 681, 309, 70, 194,
    97, 35, 682, 341, 203, 777, 358, 312, 617, 125, 307, 931, 379, 765, 875,
    951, 515, 628, 112, 659, 525, 196, 432, 134, 717, 781, 438, 440, 740,
    780, 151, 408, 487, 169, 239, 293, 467, 21, 672, 622, 557, 571, 881, 433,
    704, 376, 779, 22, 643, 460, 398, 116, 172, 503, 751, 389, 1004, 18, 576,
    415, 789, 6, 192, 696, 923, 702, 981, 892, 302, 816, 876, 880, 457, 537,
    411, 539, 716, 624, 224, 295, 406, 531, 7, 233, 478, 586, 864, 268, 974,
    338, 27, 392, 614, 839, 727, 879, 211, 250, 758, 507, 830, 129, 369, 384,
    36, 985, 12, 555, 232, 796, 221, 321, 920, 263, 42, 934, 778, 479, 761,
    939, 1006, 344, 381, 823, 44, 535, 866, 739, 752, 385, 119, 91, 566, 80,
    120, 117, 771, 675, 721, 514, 656, 271, 670, 602, 980, 850, 532, 488,
    803, 1022, 475, 801, 878, 57, 121, 991, 742, 888, 559, 105, 497, 291,
    215, 795, 236, 167, 692, 520, 272, 661, 229, 391, 814, 340, 184, 798,
    984, 773, 650, 473, 345, 558, 548, 326, 202, 145, 465, 810, 471, 158,
    813, 908, 412, 441, 964, 750, 401, 50, 915, 437, 975, 126, 979, 491, 556,
    577, 636, 685, 510, 963, 638, 367, 815, 310, 723, 349, 323, 857, 394,
    606, 505, 713, 630, 938, 106, 826, 332, 978, 599, 834, 521, 530, 248,
    883, 32, 153, 90, 754, 592, 304, 635, 775, 804, 1, 150, 836, 1013, 828,
    324, 565, 508, 113, 154, 708, 921, 703, 689, 138, 547, 911, 929, 82, 228,
    443, 468, 480, 483, 922, 135, 877, 61, 578, 111, 860, 654, 15, 331, 851,
    895, 484, 320, 218, 420, 190, 1019, 143, 362, 634, 141, 965, 10, 838,
    632, 861, 34, 722, 580, 808, 869, 554, 598, 65, 954, 787, 337, 187, 281,
    146, 563, 183, 668, 944, 171, 837, 23, 867, 541, 916, 741, 625, 123, 736,
    186, 357, 665, 977, 179, 156, 219, 220, 216, 67, 870, 902, 774, 98, 820,
    574, 613, 900, 755, 596, 370, 390, 769, 314, 701, 894, 56, 841, 949, 987,
    631, 658, 587, 204, 797, 790, 522, 745, 9, 502, 763, 86, 719, 288, 706,
    887, 728, 952, 311, 336, 446, 1002, 348, 96, 58, 199, 11, 901, 230, 833,
    188, 352, 351, 973, 3, 906, 335, 301, 266, 244, 791, 564, 619, 909, 371,
    444, 760, 657, 328, 647, 490, 425, 913, 511, 439, 540, 283, 40, 897, 849,
    60, 570, 872, 257, 749, 912, 572, 1007, 170, 407, 898, 492, 79, 747, 732,
    206, 454, 918, 375, 482, 399, 92, 748, 325, 163, 274, 405, 744, 260, 346,
    707, 626, 595, 118, 842, 136, 279, 684, 584, 101, 500, 422, 149, 956,
    1014, 493, 536, 705, 51, 914, 225, 409, 55, 822, 590, 448, 655, 205, 676,
    925, 735, 431, 784, 54, 609, 604, 39, 812, 737, 729, 466, 14, 533, 958,
    481, 770, 499, 855, 238, 182, 464, 569, 72, 947, 442, 642, 24, 87, 989,
    688, 88, 47, 762, 623, 709, 455, 817, 526, 637, 258, 84, 845, 738, 768,
    698, 423, 933, 664, 620, 607, 629, 212, 347, 249, 982, 935, 131, 89, 252,
    927, 189, 788, 853, 237, 691, 646, 403, 1010, 734, 253, 874, 807, 903,
    1020, 100, 802, 71, 799, 1003, 633, 355, 276, 300, 649, 64, 306, 161,
    608, 496, 743, 180, 485, 819, 383, 1016, 226, 308, 393, 648, 107, 19, 37,
    585, 2, 175, 645, 247, 527, 5, 419, 181, 317, 327, 519, 542, 289, 567,
    430, 579, 950, 582, 994, 1021, 583, 234, 240, 976, 41, 160, 109, 677,
    937, 210, 95, 959, 242, 753, 461, 114, 733, 368, 573, 458, 782, 605, 680,
    544, 299, 73, 652, 905, 477, 690, 93, 824, 882, 277, 946, 361, 17, 945,
    523, 472, 334, 930, 597, 603, 793, 404, 290, 942, 316, 731, 270, 960,
    936, 133, 122, 821, 966, 679, 662, 907, 282, 968, 767, 653, 20, 697, 222,
    164, 835, 30, 285, 886, 456, 436, 640, 286, 1015, 380, 840, 245, 724,
    137, 593, 173, 130, 715, 85, 885, 551, 246, 449, 103, 366, 372, 714, 313,
    865, 241, 699, 674, 374, 68, 421, 562, 292, 59, 809, 342, 651, 459, 227,
    46, 711, 764, 868, 53, 413, 278, 800, 255, 993, 318, 854, 319, 695, 315,
    469, 166, 489, 969, 730, 1001, 757, 873, 686, 197, 303, 919, 155, 673,
    940, 712, 25, 999, 63, 863, 972, 967, 785, 152, 296, 512, 402, 377, 45,
    899, 829, 354, 77, 69, 856, 417, 811, 953, 124, 418, 75, 794, 162, 414,
    1018, 568, 254, 265, 772, 588, 16, 896, 157, 889, 298, 621, 110, 844,
    1000, 108, 545, 601, 78, 862, 447, 185, 195, 818, 450, 387, 49, 805, 102,
    986, 1005, 827, 329, 28, 932, 410, 287, 435, 451, 962, 517, 48, 174, 43,
    893, 884, 261, 251, 516, 395, 910, 611, 29, 501, 223, 476, 364, 144, 871,
    998, 687, 928, 115, 453, 513, 176, 94, 168, 667, 955, 353, 434, 382, 400,
    139, 365, 996, 343, 948, 890, 1012, 663, 610, 718, 538, 1008, 639, 470,
    848, 543, 1011, 859, 671, 756, 83, 427, 159, 746, 669, 589, 971, 524,
    356, 995, 904, 256, 201, 988, 62, 397, 81, 720, 917, 209, 549, 943, 486,
    76, 148, 207, 509, 644, 386, 700, 534, 177, 550, 961, 926, 546, 428, 284,
    127, 294, 8, 269, 359, 506, 445, 997, 806, 591, 725, 178, 262, 846, 373,
    831, 504, 305, 843, 553, 378, 1017, 783, 474, 683, 581, 200, 498, 694,
    191, 217, 847, 941, 424, 235, 38, 74, 616, 786, 147, 4, 273, 214, 142,
    575, 992, 463, 983, 243, 360, 970, 350, 267, 615, 766, 494, 31, 1009,
    452, 710, 552, 128, 612, 600, 275, 322, 193
};
static_assert(std::size(GF1024_LOG) == 1024, "GF1024_EXP length should be 1024");

/* Determine the final constant to use for the specified encoding. */
uint32_t EncodingConstant(Encoding encoding) {
    assert(encoding == Encoding::BECH32 || encoding == Encoding::BECH32M);
    return encoding == Encoding::BECH32 ? 1 : 0x2bc830a3;
}

/** This function will compute what 6 5-bit values to XOR into the last 6 input values, in order to
 *  make the checksum 0. These 6 values are packed together in a single 30-bit integer. The higher
 *  bits correspond to earlier values. */
uint32_t PolyMod(const data& v)
{
    // The input is interpreted as a list of coefficients of a polynomial over F = GF(32), with an
    // implicit 1 in front. If the input is [v0,v1,v2,v3,v4], that polynomial is v(x) =
    // 1*x^5 + v0*x^4 + v1*x^3 + v2*x^2 + v3*x + v4. The implicit 1 guarantees that
    // [v0,v1,v2,...] has a distinct checksum from [0,v0,v1,v2,...].

    // The output is a 30-bit integer whose 5-bit groups are the coefficients of the remainder of
    // v(x) mod g(x), where g(x) is the Bech32 generator,
    // x^6 + {29}x^5 + {22}x^4 + {20}x^3 + {21}x^2 + {29}x + {18}. g(x) is chosen in such a way
    // that the resulting code is a BCH code, guaranteeing detection of up to 3 errors within a
    // window of 1023 characters. Among the various possible BCH codes, one was selected to in
    // fact guarantee detection of up to 4 errors within a window of 89 characters.

    // Note that the coefficients are elements of GF(32), here represented as decimal numbers
    // between {}. In this finite field, addition is just XOR of the corresponding numbers. For
    // example, {27} + {13} = {27 ^ 13} = {22}. Multiplication is more complicated, and requires
    // treating the bits of values themselves as coefficients of a polynomial over a smaller field,
    // GF(2), and multiplying those polynomials mod a^5 + a^3 + 1. For example, {5} * {26} =
    // (a^2 + 1) * (a^4 + a^3 + a) = (a^4 + a^3 + a) * a^2 + (a^4 + a^3 + a) = a^6 + a^5 + a^4 + a
    // = a^3 + 1 (mod a^5 + a^3 + 1) = {9}.

    // During the course of the loop below, `c` contains the bitpacked coefficients of the
    // polynomial constructed from just the values of v that were processed so far, mod g(x). In
    // the above example, `c` initially corresponds to 1 mod g(x), and after processing 2 inputs of
    // v, it corresponds to x^2 + v0*x + v1 mod g(x). As 1 mod g(x) = 1, that is the starting value
    // for `c`.

    // The following Sage code constructs the generator used:
    //
    // B = GF(2) # Binary field
    // BP.<b> = B[] # Polynomials over the binary field
    // F_mod = b**5 + b**3 + 1
    // F.<f> = GF(32, modulus=F_mod, repr='int') # GF(32) definition
    // FP.<x> = F[] # Polynomials over GF(32)
    // E_mod = x**2 + F.fetch_int(9)*x + F.fetch_int(23)
    // E.<e> = F.extension(E_mod) # GF(1024) extension field definition
    // for p in divisors(E.order() - 1): # Verify e has order 1023.
    //    assert((e**p == 1) == (p % 1023 == 0))
    // G = lcm([(e**i).minpoly() for i in range(997,1000)])
    // print(G) # Print out the generator
    //
    // It demonstrates that g(x) is the least common multiple of the minimal polynomials
    // of 3 consecutive powers (997,998,999) of a primitive element (e) of GF(1024).
    // That guarantees it is, in fact, the generator of a primitive BCH code with cycle
    // length 1023 and distance 4. See https://en.wikipedia.org/wiki/BCH_code for more details.

    uint32_t c = 1;
    for (const auto v_i : v) {
        // We want to update `c` to correspond to a polynomial with one extra term. If the initial
        // value of `c` consists of the coefficients of c(x) = f(x) mod g(x), we modify it to
        // correspond to c'(x) = (f(x) * x + v_i) mod g(x), where v_i is the next input to
        // process. Simplifying:
        // c'(x) = (f(x) * x + v_i) mod g(x)
        //         ((f(x) mod g(x)) * x + v_i) mod g(x)
        //         (c(x) * x + v_i) mod g(x)
        // If c(x) = c0*x^5 + c1*x^4 + c2*x^3 + c3*x^2 + c4*x + c5, we want to compute
        // c'(x) = (c0*x^5 + c1*x^4 + c2*x^3 + c3*x^2 + c4*x + c5) * x + v_i mod g(x)
        //       = c0*x^6 + c1*x^5 + c2*x^4 + c3*x^3 + c4*x^2 + c5*x + v_i mod g(x)
        //       = c0*(x^6 mod g(x)) + c1*x^5 + c2*x^4 + c3*x^3 + c4*x^2 + c5*x + v_i
        // If we call (x^6 mod g(x)) = k(x), this can be written as
        // c'(x) = (c1*x^5 + c2*x^4 + c3*x^3 + c4*x^2 + c5*x + v_i) + c0*k(x)

        // First, determine the value of c0:
        uint8_t c0 = c >> 25;

        // Then compute c1*x^5 + c2*x^4 + c3*x^3 + c4*x^2 + c5*x + v_i:
        c = ((c & 0x1ffffff) << 5) ^ v_i;

        // Finally, for each set bit n in c0, conditionally add {2^n}k(x). These constants can be
        // computed using the following Sage code (continuing the code above):
        //
        // for i in [1,2,4,8,16]: # Print out {1,2,4,8,16}*(g(x) mod x^6), packed in hex integers.
        //     v = 0
        //     for coef in reversed((F.fetch_int(i)*(G % x**6)).coefficients(sparse=True)):
        //         v = v*32 + coef.integer_representation()
        //     print("0x%x" % v)
        //
        if (c0 & 1)  c ^= 0x3b6a57b2; //     k(x) = {29}x^5 + {22}x^4 + {20}x^3 + {21}x^2 + {29}x + {18}
        if (c0 & 2)  c ^= 0x26508e6d; //  {2}k(x) = {19}x^5 +  {5}x^4 +     x^3 +  {3}x^2 + {19}x + {13}
        if (c0 & 4)  c ^= 0x1ea119fa; //  {4}k(x) = {15}x^5 + {10}x^4 +  {2}x^3 +  {6}x^2 + {15}x + {26}
        if (c0 & 8)  c ^= 0x3d4233dd; //  {8}k(x) = {30}x^5 + {20}x^4 +  {4}x^3 + {12}x^2 + {30}x + {29}
        if (c0 & 16) c ^= 0x2a1462b3; // {16}k(x) = {21}x^5 +     x^4 +  {8}x^3 + {24}x^2 + {21}x + {19}

    }
    return c;
}

/** Syndrome computes the values s_j = R(e^j) for j in [997, 998, 999]. As described above, the
 * generator polynomial G is the LCM of the minimal polynomials of (e)^997, (e)^998, and (e)^999.
 *
 * Consider a codeword with errors, of the form R(x) = C(x) + E(x). The residue is the bit-packed
 * result of computing R(x) mod G(X), where G is the generator of the code. Because C(x) is a valid
 * codeword, it is a multiple of G(X), so the residue is in fact just E(x) mod G(x). Note that all
 * of the (e)^j are roots of G(x) by definition, so R((e)^j) = E((e)^j).
 *
 * Syndrome returns the three values packed into a 30-bit integer, where each 10 bits is one value.
 */
uint32_t Syndrome(const uint32_t residue) {
    // Let R(x) = r1*x^5 + r2*x^4 + r3*x^3 + r4*x^2 + r5*x + r6
    // low is the first 5 bits, corresponding to the r6 in the residue
    // (the constant term of the polynomial).

    uint32_t low = residue & 0x1f;

    // Recall that XOR corresponds to addition in a characteristic 2 field.
    //
    // To compute R((e)^j), we are really computing:
    // r1*(e)^(j*5) + r2*(e)^(j*4) + r3*(e)^(j*3) + r4*(e)^(j*2) + r5*(e)^j + r6
    // Now note that all of the (e)^(j*i) for i in [5..0] are constants and can be precomputed
    // for efficiency. But even more than that, we can consider each coefficient as a bit-string.
    // For example, take r5 = (b_5, b_4, b_3, b_2, b_1) written out as 5 bits. Then:
    // r5*(e)^j = b_1*(e)^j + b_2*(2*(e)^j) + b_3*(4*(e)^j) + b_4*(8*(e)^j) + b_5*(16*(e)^j)
    // where all the (2^i*(e)^j) are constants and can be precomputed. Then we just add each
    // of these corresponding constants to our final value based on the bit values b_i.
    // This is exactly what is done below. Note that all three values of s_j for j in (997, 998,
    // 999) are computed simultaneously.
    //
    // We begin by setting s_j = low = r6 for all three values of j, because these are unconditional.
    // Then for each following bit, we add the corresponding precomputed constant if the bit is 1.
    // For example, 0x31edd3c4 is 1100011110 1101110100 1111000100 when unpacked in groups of 10
    // bits, corresponding exactly to a^999 || a^998 || a^997 (matching the corresponding values in
    // GF1024_EXP above).
    //
    // The following sage code reproduces these constants:
    // for k in range(1, 6):
    //     for b in [1,2,4,8,16]:
    //         c0 = GF1024_EXP[(997*k + GF1024_LOG[b]) % 1023]
    //         c1 = GF1024_EXP[(998*k + GF1024_LOG[b]) % 1023]
    //         c2 = GF1024_EXP[(999*k + GF1024_LOG[b]) % 1023]
    //         c = c2 << 20 | c1 << 10 | c0
    //         print("0x%x" % c)

    return low ^ (low << 10) ^ (low << 20) ^
        ((residue >> 5) & 1 ? 0x31edd3c4 : 0) ^
        ((residue >> 6) & 1 ? 0x335f86a8 : 0) ^
        ((residue >> 7) & 1 ? 0x363b8870 : 0) ^
        ((residue >> 8) & 1 ? 0x3e6390c9 : 0) ^
        ((residue >> 9) & 1 ? 0x2ec72192 : 0) ^
        ((residue >> 10) & 1 ? 0x1046f79d : 0) ^
        ((residue >> 11) & 1 ? 0x208d4e33 : 0) ^
        ((residue >> 12) & 1 ? 0x130ebd6f : 0) ^
        ((residue >> 13) & 1 ? 0x2499fade : 0) ^
        ((residue >> 14) & 1 ? 0x1b27d4b5 : 0) ^
        ((residue >> 15) & 1 ? 0x04be1eb4 : 0) ^
        ((residue >> 16) & 1 ? 0x0968b861 : 0) ^
        ((residue >> 17) & 1 ? 0x1055f0c2 : 0) ^
        ((residue >> 18) & 1 ? 0x20ab4584 : 0) ^
        ((residue >> 19) & 1 ? 0x1342af08 : 0) ^
        ((residue >> 20) & 1 ? 0x24f1f318 : 0) ^
        ((residue >> 21) & 1 ? 0x1be34739 : 0) ^
        ((residue >> 22) & 1 ? 0x35562f7b : 0) ^
        ((residue >> 23) & 1 ? 0x3a3c5bff : 0) ^
        ((residue >> 24) & 1 ? 0x266c96f7 : 0) ^
        ((residue >> 25) & 1 ? 0x25c78b65 : 0) ^
        ((residue >> 26) & 1 ? 0x1b1f13ea : 0) ^
        ((residue >> 27) & 1 ? 0x34baa2f4 : 0) ^
        ((residue >> 28) & 1 ? 0x3b61c0e1 : 0) ^
        ((residue >> 29) & 1 ? 0x265325c2 : 0);
}

/** Convert to lower case. */
inline unsigned char LowerCase(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A') + 'a' : c;
}

void push_range(int from, int to, std::vector<int>& vec)
{
    for (int i = from; i < to; i++) {
        vec.push_back(i);
    }
}

/** Return index of first invalid character in a Bech32 string. */
bool CheckCharacters(const std::string& str, std::vector<int>& errors) {
    bool lower = false, upper = false;
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char c = str[i];
        if (c >= 'a' && c <= 'z') {
            if (upper) {
                errors.push_back(i);
            } else {
                lower = true;
            }
        } else if (c >= 'A' && c <= 'Z') {
            if (lower) {
                errors.push_back(i);
            } else {
                upper = true;
            }
        } else if (c < 33 || c > 126) {
            errors.push_back(i);
        }
    }
    return errors.empty();
}

/** Expand a HRP for use in checksum computation. */
data ExpandHRP(const std::string& hrp)
{
    data ret;
    ret.reserve(hrp.size() + 90);
    ret.resize(hrp.size() * 2 + 1);
    for (size_t i = 0; i < hrp.size(); ++i) {
        unsigned char c = hrp[i];
        ret[i] = c >> 5;
        ret[i + hrp.size() + 1] = c & 0x1f;
    }
    ret[hrp.size()] = 0;
    return ret;
}

/** Verify a checksum. */
Encoding VerifyChecksum(const std::string& hrp, const data& values)
{
    // PolyMod computes what value to xor into the final values to make the checksum 0. However,
    // if we required that the checksum was 0, it would be the case that appending a 0 to a valid
    // list of values would result in a new valid list. For that reason, Bech32 requires the
    // resulting checksum to be 1 instead. In Bech32m, this constant was amended. See
    // https://gist.github.com/sipa/14c248c288c3880a3b191f978a34508e for details.
    const uint32_t check = PolyMod(Cat(ExpandHRP(hrp), values));
    if (check == EncodingConstant(Encoding::BECH32)) return Encoding::BECH32;
    if (check == EncodingConstant(Encoding::BECH32M)) return Encoding::BECH32M;
    return Encoding::INVALID;
}

/** Create a checksum. */
data CreateChecksum(Encoding encoding, const std::string& hrp, const data& values)
{
    data enc = Cat(ExpandHRP(hrp), values);
    enc.resize(enc.size() + 6); // Append 6 zeroes
    uint32_t mod = PolyMod(enc) ^ EncodingConstant(encoding); // Determine what to XOR into those 6 zeroes.
    data ret(6);
    for (size_t i = 0; i < 6; ++i) {
        // Convert the 5-bit groups in mod to checksum values.
        ret[i] = (mod >> (5 * (5 - i))) & 31;
    }
    return ret;
}

} // namespace

/** Encode a Bech32 or Bech32m string. */
std::string Encode(Encoding encoding, const std::string& hrp, const data& values) {
    // First ensure that the HRP is all lowercase. BIP-173 and BIP350 require an encoder
    // to return a lowercase Bech32/Bech32m string, but if given an uppercase HRP, the
    // result will always be invalid.
    for (const char& c : hrp) assert(c < 'A' || c > 'Z');
    data checksum = CreateChecksum(encoding, hrp, values);
    data combined = Cat(values, checksum);
    std::string ret = hrp + '1';
    ret.reserve(ret.size() + combined.size());
    for (const auto c : combined) {
        ret += CHARSET[c];
    }
    return ret;
}

/** Decode a Bech32 or Bech32m string. */
DecodeResult Decode(const std::string& str) {
    std::vector<int> errors;
    if (!CheckCharacters(str, errors)) return {};
    size_t pos = str.rfind('1');
    if (str.size() > 90 || pos == str.npos || pos == 0 || pos + 7 > str.size()) {
        return {};
    }
    data values(str.size() - 1 - pos);
    for (size_t i = 0; i < str.size() - 1 - pos; ++i) {
        unsigned char c = str[i + pos + 1];
        int8_t rev = CHARSET_REV[c];

        if (rev == -1) {
            return {};
        }
        values[i] = rev;
    }
    std::string hrp;
    for (size_t i = 0; i < pos; ++i) {
        hrp += LowerCase(str[i]);
    }
    Encoding result = VerifyChecksum(hrp, values);
    if (result == Encoding::INVALID) return {};
    return {result, std::move(hrp), data(values.begin(), values.end() - 6)};
}

/** Find index of an incorrect character in a Bech32 string. */
std::string LocateErrors(const std::string& str, std::vector<int>& error_locations) {
    if (str.size() > 90) {
        push_range(90, str.size(), error_locations);
        return "Bech32 string too long";
    }
    if (!CheckCharacters(str, error_locations)){
        return "Invalid character or mixed case";
    }
    size_t pos = str.rfind('1');
    if (pos == str.npos) {
        return "Missing separator";
    }
    if (pos == 0 || pos + 7 > str.size()) {
        error_locations.push_back(pos);
        return "Invalid separator position";
    }
    std::string hrp;
    for (size_t i = 0; i < pos; ++i) {
        hrp += LowerCase(str[i]);
    }

    size_t length = str.size() - 1 - pos; // length of data part
    data values(length);
    for (size_t i = pos + 1; i < str.size(); ++i) {
        unsigned char c = str[i];
        int8_t rev = CHARSET_REV[c];
        if (rev == -1) {
            error_locations.push_back(i);
            return "Invalid Base 32 character";
        }
        values[i - pos - 1] = rev;
    }

    // We attempt error detection with both bech32 and bech32m, and choose the one with the fewest errors
    // We can't simply use the segwit version, because that may be one of the errors
    for (Encoding encoding : {Encoding::BECH32, Encoding::BECH32M}) {
        std::vector<int> possible_errors;
        // Recall that (ExpandHRP(hrp) ++ values) is interpreted as a list of coefficients of a polynomial
        // over GF(32). PolyMod computes the "remainder" of this polynomial modulo the generator G(x).
        uint32_t residue = PolyMod(Cat(ExpandHRP(hrp), values)) ^ EncodingConstant(encoding);

        // All valid codewords should be multiples of G(x), so this remainder (after XORing with the encoding
        // constant) should be 0 - hence 0 indicates there are no errors present.
        if (residue != 0) {
            // If errors are present, our polynomial must be of the form C(x) + E(x) where C is the valid
            // codeword (a multiple of G(x)), and E encodes the errors.
            uint32_t syn = Syndrome(residue);

            // Unpack the three 10-bit syndrome values
            int s0 = syn & 0x3FF;
            int s1 = (syn >> 10) & 0x3FF;
            int s2 = syn >> 20;

            // Get the discrete logs of these values in GF1024 for more efficient computation
            int l_s0 = GF1024_LOG[s0];
            int l_s1 = GF1024_LOG[s1];
            int l_s2 = GF1024_LOG[s2];

            // First, suppose there is only a single error. Then E(x) = e1*x^p1 for some position p1
            // Then s0 = E((e)^997) = e1*(e)^(997*p1) and s1 = E((e)^998) = e1*(e)^(998*p1)
            // Therefore s1/s0 = (e)^p1, and by the same logic, s2/s1 = (e)^p1 too.
            // Hence, s1^2 == s0*s2, which is exactly the condition we check first:
            if (l_s0 != -1 && l_s1 != -1 && l_s2 != -1 && (2 * l_s1 - l_s2 - l_s0 + 2046) % 1023 == 0) {
                // Compute the error position p1 as l_s1 - l_s0 = p1 (mod 1023)
                size_t p1 = (l_s1 - l_s0 + 1023) % 1023; // the +1023 ensures it is positive
                // Now because s0 = e1*(e)^(997*p1), we get e1 = s0/((e)^(997*p1)). Remember that (e)^1023 = 1,
                // so 1/((e)^997) = (e)^(1023-997).
                int l_e1 = l_s0 + (1023 - 997) * p1;
                // Finally, some sanity checks on the result:
                // - The error position should be within the length of the data
                // - e1 should be in GF(32), which implies that e1 = (e)^(33k) for some k (the 31 non-zero elements
                // of GF(32) form an index 33 subgroup of the 1023 non-zero elements of GF(1024)).
                if (p1 < length && !(l_e1 % 33)) {
                    // Polynomials run from highest power to lowest, so the index p1 is from the right.
                    // We don't return e1 because it is dangerous to suggest corrections to the user,
                    // the user should check the address themselves.
                    possible_errors.push_back(str.size() - p1 - 1);
                }
            // Otherwise, suppose there are two errors. Then E(x) = e1*x^p1 + e2*x^p2.
            } else {
                // For all possible first error positions p1
                for (size_t p1 = 0; p1 < length; ++p1) {
                    // We have guessed p1, and want to solve for p2. Recall that E(x) = e1*x^p1 + e2*x^p2, so
                    // s0 = E((e)^997) = e1*(e)^(997^p1) + e2*(e)^(997*p2), and similar for s1 and s2.
                    //
                    // Consider s2 + s1*(e)^p1
                    //          = 2e1*(e)^(999^p1) + e2*(e)^(999*p2) + e2*(e)^(998*p2)*(e)^p1
                    //          = e2*(e)^(999*p2) + e2*(e)^(998*p2)*(e)^p1
                    //    (Because we are working in characteristic 2.)
                    //          = e2*(e)^(998*p2) ((e)^p2 + (e)^p1)
                    //
                    int s2_s1p1 = s2 ^ (s1 == 0 ? 0 : GF1024_EXP[(l_s1 + p1) % 1023]);
                    if (s2_s1p1 == 0) continue;
                    int l_s2_s1p1 = GF1024_LOG[s2_s1p1];

                    // Similarly, s1 + s0*(e)^p1
                    //          = e2*(e)^(997*p2) ((e)^p2 + (e)^p1)
                    int s1_s0p1 = s1 ^ (s0 == 0 ? 0 : GF1024_EXP[(l_s0 + p1) % 1023]);
                    if (s1_s0p1 == 0) continue;
                    int l_s1_s0p1 = GF1024_LOG[s1_s0p1];

                    // So, putting these together, we can compute the second error position as
                    // (e)^p2 = (s2 + s1^p1)/(s1 + s0^p1)
                    // p2 = log((e)^p2)
                    size_t p2 = (l_s2_s1p1 - l_s1_s0p1 + 1023) % 1023;

                    // Sanity checks that p2 is a valid position and not the same as p1
                    if (p2 >= length || p1 == p2) continue;

                    // Now we want to compute the error values e1 and e2.
                    // Similar to above, we compute s1 + s0*(e)^p2
                    //          = e1*(e)^(997*p1) ((e)^p1 + (e)^p2)
                    int s1_s0p2 = s1 ^ (s0 == 0 ? 0 : GF1024_EXP[(l_s0 + p2) % 1023]);
                    if (s1_s0p2 == 0) continue;
                    int l_s1_s0p2 = GF1024_LOG[s1_s0p2];

                    // And compute (the log of) 1/((e)^p1 + (e)^p2))
                    int inv_p1_p2 = 1023 - GF1024_LOG[GF1024_EXP[p1] ^ GF1024_EXP[p2]];

                    // Then (s1 + s0*(e)^p1) * (1/((e)^p1 + (e)^p2)))
                    //         = e2*(e)^(997*p2)
                    // Then recover e2 by dividing by (e)^(997*p2)
                    int l_e2 = l_s1_s0p1 + inv_p1_p2 + (1023 - 997) * p2;
                    // Check that e2 is in GF(32)
                    if (l_e2 % 33) continue;

                    // In the same way, (s1 + s0*(e)^p2) * (1/((e)^p1 + (e)^p2)))
                    //         = e1*(e)^(997*p1)
                    // So recover e1 by dividing by (e)^(997*p1)
                    int l_e1 = l_s1_s0p2 + inv_p1_p2 + (1023 - 997) * p1;
                    // Check that e1 is in GF(32)
                    if (l_e1 % 33) continue;

                    // Again, we do not return e1 or e2 for safety.
                    // Order the error positions from the left of the string and return them
                    if (p1 > p2) {
                        possible_errors.push_back(str.size() - p1 - 1);
                        possible_errors.push_back(str.size() - p2 - 1);
                    } else {
                        possible_errors.push_back(str.size() - p2 - 1);
                        possible_errors.push_back(str.size() - p1 - 1);
                    }
                    break;
                }
            }
        } else {
            // No errors
            error_locations.clear();
            return "";
        }

        if (error_locations.empty() || (!possible_errors.empty() && possible_errors.size() < error_locations.size())) {
            error_locations = std::move(possible_errors);
        }
    }
    return "Invalid checksum";
}

} // namespace bech32
