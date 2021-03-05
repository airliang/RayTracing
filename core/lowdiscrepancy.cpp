#include "lowdiscrepancy.h"
#include "log.h"

namespace AIR
{
	// Low Discrepancy Data Definitions
	//存储了PrimeTableSize个质数的数组
	const int Primes[PrimeTableSize] = {
		2, 3, 5, 7, 11,
		// Subsequent prime numbers
		13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89,
		97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167,
		173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251,
		257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347,
		349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433,
		439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523,
		541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619,
		631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727,
		733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827,
		829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937,
		941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031,
		1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103,
		1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201,
		1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289,
		1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381,
		1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471,
		1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553,
		1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621,
		1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723,
		1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823,
		1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913,
		1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011,
		2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099,
		2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207,
		2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293,
		2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381,
		2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467,
		2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591,
		2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683,
		2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749,
		2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843,
		2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953,
		2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049,
		3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169,
		3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259,
		3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359,
		3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463,
		3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547,
		3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637,
		3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733,
		3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847,
		3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929,
		3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027,
		4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133,
		4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241,
		4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339,
		4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451,
		4457, 4463, 4481, 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547, 4549,
		4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651,
		4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759,
		4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877,
		4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969,
		4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059,
		5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171,
		5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281,
		5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407,
		5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483,
		5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581,
		5591, 5623, 5639, 5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689,
		5693, 5701, 5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801,
		5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857, 5861, 5867, 5869,
		5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007, 6011,
		6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113,
		6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217,
		6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311,
		6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389,
		6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547,
		6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653,
		6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737,
		6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841,
		6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959,
		6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039,
		7043, 7057, 7069, 7079, 7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177,
		7187, 7193, 7207, 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283,
		7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417,
		7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517, 7523,
		7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589, 7591,
		7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691, 7699,
		7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823,
		7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919 };

	//每个质数的和前面所有质数的和在数组中的起始位置索引
	const int PrimeSums[PrimeTableSize] = {
		0, 2, 5, 10, 17,
		// Subsequent prime sums
		28, 41, 58, 77, 100, 129, 160, 197, 238, 281, 328, 381, 440, 501, 568, 639,
		712, 791, 874, 963, 1060, 1161, 1264, 1371, 1480, 1593, 1720, 1851, 1988,
		2127, 2276, 2427, 2584, 2747, 2914, 3087, 3266, 3447, 3638, 3831, 4028,
		4227, 4438, 4661, 4888, 5117, 5350, 5589, 5830, 6081, 6338, 6601, 6870,
		7141, 7418, 7699, 7982, 8275, 8582, 8893, 9206, 9523, 9854, 10191, 10538,
		10887, 11240, 11599, 11966, 12339, 12718, 13101, 13490, 13887, 14288, 14697,
		15116, 15537, 15968, 16401, 16840, 17283, 17732, 18189, 18650, 19113, 19580,
		20059, 20546, 21037, 21536, 22039, 22548, 23069, 23592, 24133, 24680, 25237,
		25800, 26369, 26940, 27517, 28104, 28697, 29296, 29897, 30504, 31117, 31734,
		32353, 32984, 33625, 34268, 34915, 35568, 36227, 36888, 37561, 38238, 38921,
		39612, 40313, 41022, 41741, 42468, 43201, 43940, 44683, 45434, 46191, 46952,
		47721, 48494, 49281, 50078, 50887, 51698, 52519, 53342, 54169, 54998, 55837,
		56690, 57547, 58406, 59269, 60146, 61027, 61910, 62797, 63704, 64615, 65534,
		66463, 67400, 68341, 69288, 70241, 71208, 72179, 73156, 74139, 75130, 76127,
		77136, 78149, 79168, 80189, 81220, 82253, 83292, 84341, 85392, 86453, 87516,
		88585, 89672, 90763, 91856, 92953, 94056, 95165, 96282, 97405, 98534, 99685,
		100838, 102001, 103172, 104353, 105540, 106733, 107934, 109147, 110364,
		111587, 112816, 114047, 115284, 116533, 117792, 119069, 120348, 121631,
		122920, 124211, 125508, 126809, 128112, 129419, 130738, 132059, 133386,
		134747, 136114, 137487, 138868, 140267, 141676, 143099, 144526, 145955,
		147388, 148827, 150274, 151725, 153178, 154637, 156108, 157589, 159072,
		160559, 162048, 163541, 165040, 166551, 168074, 169605, 171148, 172697,
		174250, 175809, 177376, 178947, 180526, 182109, 183706, 185307, 186914,
		188523, 190136, 191755, 193376, 195003, 196640, 198297, 199960, 201627,
		203296, 204989, 206686, 208385, 210094, 211815, 213538, 215271, 217012,
		218759, 220512, 222271, 224048, 225831, 227618, 229407, 231208, 233019,
		234842, 236673, 238520, 240381, 242248, 244119, 245992, 247869, 249748,
		251637, 253538, 255445, 257358, 259289, 261222, 263171, 265122, 267095,
		269074, 271061, 273054, 275051, 277050, 279053, 281064, 283081, 285108,
		287137, 289176, 291229, 293292, 295361, 297442, 299525, 301612, 303701,
		305800, 307911, 310024, 312153, 314284, 316421, 318562, 320705, 322858,
		325019, 327198, 329401, 331608, 333821, 336042, 338279, 340518, 342761,
		345012, 347279, 349548, 351821, 354102, 356389, 358682, 360979, 363288,
		365599, 367932, 370271, 372612, 374959, 377310, 379667, 382038, 384415,
		386796, 389179, 391568, 393961, 396360, 398771, 401188, 403611, 406048,
		408489, 410936, 413395, 415862, 418335, 420812, 423315, 425836, 428367,
		430906, 433449, 435998, 438549, 441106, 443685, 446276, 448869, 451478,
		454095, 456716, 459349, 461996, 464653, 467312, 469975, 472646, 475323,
		478006, 480693, 483382, 486075, 488774, 491481, 494192, 496905, 499624,
		502353, 505084, 507825, 510574, 513327, 516094, 518871, 521660, 524451,
		527248, 530049, 532852, 535671, 538504, 541341, 544184, 547035, 549892,
		552753, 555632, 558519, 561416, 564319, 567228, 570145, 573072, 576011,
		578964, 581921, 584884, 587853, 590824, 593823, 596824, 599835, 602854,
		605877, 608914, 611955, 615004, 618065, 621132, 624211, 627294, 630383,
		633492, 636611, 639732, 642869, 646032, 649199, 652368, 655549, 658736,
		661927, 665130, 668339, 671556, 674777, 678006, 681257, 684510, 687767,
		691026, 694297, 697596, 700897, 704204, 707517, 710836, 714159, 717488,
		720819, 724162, 727509, 730868, 734229, 737600, 740973, 744362, 747753,
		751160, 754573, 758006, 761455, 764912, 768373, 771836, 775303, 778772,
		782263, 785762, 789273, 792790, 796317, 799846, 803379, 806918, 810459,
		814006, 817563, 821122, 824693, 828274, 831857, 835450, 839057, 842670,
		846287, 849910, 853541, 857178, 860821, 864480, 868151, 871824, 875501,
		879192, 882889, 886590, 890299, 894018, 897745, 901478, 905217, 908978,
		912745, 916514, 920293, 924086, 927883, 931686, 935507, 939330, 943163,
		947010, 950861, 954714, 958577, 962454, 966335, 970224, 974131, 978042,
		981959, 985878, 989801, 993730, 997661, 1001604, 1005551, 1009518, 1013507,
		1017508, 1021511, 1025518, 1029531, 1033550, 1037571, 1041598, 1045647,
		1049698, 1053755, 1057828, 1061907, 1065998, 1070091, 1074190, 1078301,
		1082428, 1086557, 1090690, 1094829, 1098982, 1103139, 1107298, 1111475,
		1115676, 1119887, 1124104, 1128323, 1132552, 1136783, 1141024, 1145267,
		1149520, 1153779, 1158040, 1162311, 1166584, 1170867, 1175156, 1179453,
		1183780, 1188117, 1192456, 1196805, 1201162, 1205525, 1209898, 1214289,
		1218686, 1223095, 1227516, 1231939, 1236380, 1240827, 1245278, 1249735,
		1254198, 1258679, 1263162, 1267655, 1272162, 1276675, 1281192, 1285711,
		1290234, 1294781, 1299330, 1303891, 1308458, 1313041, 1317632, 1322229,
		1326832, 1331453, 1336090, 1340729, 1345372, 1350021, 1354672, 1359329,
		1363992, 1368665, 1373344, 1378035, 1382738, 1387459, 1392182, 1396911,
		1401644, 1406395, 1411154, 1415937, 1420724, 1425513, 1430306, 1435105,
		1439906, 1444719, 1449536, 1454367, 1459228, 1464099, 1468976, 1473865,
		1478768, 1483677, 1488596, 1493527, 1498460, 1503397, 1508340, 1513291,
		1518248, 1523215, 1528184, 1533157, 1538144, 1543137, 1548136, 1553139,
		1558148, 1563159, 1568180, 1573203, 1578242, 1583293, 1588352, 1593429,
		1598510, 1603597, 1608696, 1613797, 1618904, 1624017, 1629136, 1634283,
		1639436, 1644603, 1649774, 1654953, 1660142, 1665339, 1670548, 1675775,
		1681006, 1686239, 1691476, 1696737, 1702010, 1707289, 1712570, 1717867,
		1723170, 1728479, 1733802, 1739135, 1744482, 1749833, 1755214, 1760601,
		1765994, 1771393, 1776800, 1782213, 1787630, 1793049, 1798480, 1803917,
		1809358, 1814801, 1820250, 1825721, 1831198, 1836677, 1842160, 1847661,
		1853164, 1858671, 1864190, 1869711, 1875238, 1880769, 1886326, 1891889,
		1897458, 1903031, 1908612, 1914203, 1919826, 1925465, 1931106, 1936753,
		1942404, 1948057, 1953714, 1959373, 1965042, 1970725, 1976414, 1982107,
		1987808, 1993519, 1999236, 2004973, 2010714, 2016457, 2022206, 2027985,
		2033768, 2039559, 2045360, 2051167, 2056980, 2062801, 2068628, 2074467,
		2080310, 2086159, 2092010, 2097867, 2103728, 2109595, 2115464, 2121343,
		2127224, 2133121, 2139024, 2144947, 2150874, 2156813, 2162766, 2168747,
		2174734, 2180741, 2186752, 2192781, 2198818, 2204861, 2210908, 2216961,
		2223028, 2229101, 2235180, 2241269, 2247360, 2253461, 2259574, 2265695,
		2271826, 2277959, 2284102, 2290253, 2296416, 2302589, 2308786, 2314985,
		2321188, 2327399, 2333616, 2339837, 2346066, 2352313, 2358570, 2364833,
		2371102, 2377373, 2383650, 2389937, 2396236, 2402537, 2408848, 2415165,
		2421488, 2427817, 2434154, 2440497, 2446850, 2453209, 2459570, 2465937,
		2472310, 2478689, 2485078, 2491475, 2497896, 2504323, 2510772, 2517223,
		2523692, 2530165, 2536646, 2543137, 2549658, 2556187, 2562734, 2569285,
		2575838, 2582401, 2588970, 2595541, 2602118, 2608699, 2615298, 2621905,
		2628524, 2635161, 2641814, 2648473, 2655134, 2661807, 2668486, 2675175,
		2681866, 2688567, 2695270, 2701979, 2708698, 2715431, 2722168, 2728929,
		2735692, 2742471, 2749252, 2756043, 2762836, 2769639, 2776462, 2783289,
		2790118, 2796951, 2803792, 2810649, 2817512, 2824381, 2831252, 2838135,
		2845034, 2851941, 2858852, 2865769, 2872716, 2879665, 2886624, 2893585,
		2900552, 2907523, 2914500, 2921483, 2928474, 2935471, 2942472, 2949485,
		2956504, 2963531, 2970570, 2977613, 2984670, 2991739, 2998818, 3005921,
		3013030, 3020151, 3027278, 3034407, 3041558, 3048717, 3055894, 3063081,
		3070274, 3077481, 3084692, 3091905, 3099124, 3106353, 3113590, 3120833,
		3128080, 3135333, 3142616, 3149913, 3157220, 3164529, 3171850, 3179181,
		3186514, 3193863, 3201214, 3208583, 3215976, 3223387, 3230804, 3238237,
		3245688, 3253145, 3260604, 3268081, 3275562, 3283049, 3290538, 3298037,
		3305544, 3313061, 3320584, 3328113, 3335650, 3343191, 3350738, 3358287,
		3365846, 3373407, 3380980, 3388557, 3396140, 3403729, 3411320, 3418923,
		3426530, 3434151, 3441790, 3449433, 3457082, 3464751, 3472424, 3480105,
		3487792, 3495483, 3503182, 3510885, 3518602, 3526325, 3534052, 3541793,
		3549546, 3557303, 3565062, 3572851, 3580644, 3588461, 3596284, 3604113,
		3611954, 3619807, 3627674, 3635547, 3643424, 3651303, 3659186, 3667087,
		3674994,
	};

	// Low Discrepancy Static Functions
	template <int base>
	static Float RadicalInverseSpecialized(uint64_t a) 
	{
		const Float invBase = (Float)1 / (Float)base;
		uint64_t reversedDigits = 0;
		Float invBaseN = 1;
		while (a) 
		{
			uint64_t next = a / base;
			uint64_t digit = a - next * base;
			reversedDigits = reversedDigits * base + digit;
			invBaseN *= invBase;
			a = next;
		}
		//DCHECK_LT(reversedDigits * invBaseN, 1.00001);
		return std::min(reversedDigits * invBaseN, OneMinusEpsilon);
	}

	template <int base>
	NO_INLINE static Float
		ScrambledRadicalInverseSpecialized(const uint16_t* perm, uint64_t a) 
	{
		const Float invBase = (Float)1 / (Float)base;
		uint64_t reversedDigits = 0;
		Float invBaseN = 1;
		while (a) 
		{
			uint64_t next = a / base;
			uint64_t digit = a - next * base;
			//CHECK_LT(perm[digit], base);
			reversedDigits = reversedDigits * base + perm[digit];
			invBaseN *= invBase;
			a = next;
		}
		//DCHECK_LT(invBaseN * (reversedDigits + invBase * perm[0] / (1 - invBase)),
		//	1.00001);
		return std::min(
			invBaseN * (reversedDigits + invBase * perm[0] / (1 - invBase)),
			OneMinusEpsilon);
	}

	Float RadicalInverse(int baseIndex, uint64_t a)
	{
		switch (baseIndex)
		{
		case 0:
			// Compute base-2 radical inverse
//#ifndef PBRT_HAVE_HEX_FP_CONSTANTS
//			return ReverseBits64(a) * 5.4210108624275222e-20;
//#else
			return ReverseBits64(a) * 0x1p-64;
			//#endif
		case 1:
			return RadicalInverseSpecialized<3>(a);
		case 2:
			return RadicalInverseSpecialized<5>(a);
		case 3:
			return RadicalInverseSpecialized<7>(a);
		case 4:
			return RadicalInverseSpecialized<11>(a);
		case 5:
			return RadicalInverseSpecialized<13>(a);
		case 6:
			return RadicalInverseSpecialized<17>(a);
		case 7:
			return RadicalInverseSpecialized<19>(a);
		case 8:
			return RadicalInverseSpecialized<23>(a);
		case 9:
			return RadicalInverseSpecialized<29>(a);
		case 10:
			return RadicalInverseSpecialized<31>(a);
		case 11:
			return RadicalInverseSpecialized<37>(a);
		case 12:
			return RadicalInverseSpecialized<41>(a);
		case 13:
			return RadicalInverseSpecialized<43>(a);
		case 14:
			return RadicalInverseSpecialized<47>(a);
		case 15:
			return RadicalInverseSpecialized<53>(a);
		case 16:
			return RadicalInverseSpecialized<59>(a);
		case 17:
			return RadicalInverseSpecialized<61>(a);
		case 18:
			return RadicalInverseSpecialized<67>(a);
		case 19:
			return RadicalInverseSpecialized<71>(a);
		case 20:
			return RadicalInverseSpecialized<73>(a);
		case 21:
			return RadicalInverseSpecialized<79>(a);
		case 22:
			return RadicalInverseSpecialized<83>(a);
		case 23:
			return RadicalInverseSpecialized<89>(a);
		case 24:
			return RadicalInverseSpecialized<97>(a);
		case 25:
			return RadicalInverseSpecialized<101>(a);
		case 26:
			return RadicalInverseSpecialized<103>(a);
		case 27:
			return RadicalInverseSpecialized<107>(a);
		case 28:
			return RadicalInverseSpecialized<109>(a);
		case 29:
			return RadicalInverseSpecialized<113>(a);
		case 30:
			return RadicalInverseSpecialized<127>(a);
		case 31:
			return RadicalInverseSpecialized<131>(a);
		case 32:
			return RadicalInverseSpecialized<137>(a);
		case 33:
			return RadicalInverseSpecialized<139>(a);
		case 34:
			return RadicalInverseSpecialized<149>(a);
		case 35:
			return RadicalInverseSpecialized<151>(a);
		case 36:
			return RadicalInverseSpecialized<157>(a);
		case 37:
			return RadicalInverseSpecialized<163>(a);
		case 38:
			return RadicalInverseSpecialized<167>(a);
		case 39:
			return RadicalInverseSpecialized<173>(a);
		case 40:
			return RadicalInverseSpecialized<179>(a);
		case 41:
			return RadicalInverseSpecialized<181>(a);
		case 42:
			return RadicalInverseSpecialized<191>(a);
		case 43:
			return RadicalInverseSpecialized<193>(a);
		case 44:
			return RadicalInverseSpecialized<197>(a);
		case 45:
			return RadicalInverseSpecialized<199>(a);
		case 46:
			return RadicalInverseSpecialized<211>(a);
		case 47:
			return RadicalInverseSpecialized<223>(a);
		case 48:
			return RadicalInverseSpecialized<227>(a);
		case 49:
			return RadicalInverseSpecialized<229>(a);
		case 50:
			return RadicalInverseSpecialized<233>(a);
		case 51:
			return RadicalInverseSpecialized<239>(a);
		case 52:
			return RadicalInverseSpecialized<241>(a);
		case 53:
			return RadicalInverseSpecialized<251>(a);
		case 54:
			return RadicalInverseSpecialized<257>(a);
		case 55:
			return RadicalInverseSpecialized<263>(a);
		case 56:
			return RadicalInverseSpecialized<269>(a);
		case 57:
			return RadicalInverseSpecialized<271>(a);
		case 58:
			return RadicalInverseSpecialized<277>(a);
		case 59:
			return RadicalInverseSpecialized<281>(a);
		case 60:
			return RadicalInverseSpecialized<283>(a);
		case 61:
			return RadicalInverseSpecialized<293>(a);
		case 62:
			return RadicalInverseSpecialized<307>(a);
		case 63:
			return RadicalInverseSpecialized<311>(a);
		case 64:
			return RadicalInverseSpecialized<313>(a);
		case 65:
			return RadicalInverseSpecialized<317>(a);
		case 66:
			return RadicalInverseSpecialized<331>(a);
		case 67:
			return RadicalInverseSpecialized<337>(a);
		case 68:
			return RadicalInverseSpecialized<347>(a);
		case 69:
			return RadicalInverseSpecialized<349>(a);
		case 70:
			return RadicalInverseSpecialized<353>(a);
		case 71:
			return RadicalInverseSpecialized<359>(a);
		case 72:
			return RadicalInverseSpecialized<367>(a);
		case 73:
			return RadicalInverseSpecialized<373>(a);
		case 74:
			return RadicalInverseSpecialized<379>(a);
		case 75:
			return RadicalInverseSpecialized<383>(a);
		case 76:
			return RadicalInverseSpecialized<389>(a);
		case 77:
			return RadicalInverseSpecialized<397>(a);
		case 78:
			return RadicalInverseSpecialized<401>(a);
		case 79:
			return RadicalInverseSpecialized<409>(a);
		case 80:
			return RadicalInverseSpecialized<419>(a);
		case 81:
			return RadicalInverseSpecialized<421>(a);
		case 82:
			return RadicalInverseSpecialized<431>(a);
		case 83:
			return RadicalInverseSpecialized<433>(a);
		case 84:
			return RadicalInverseSpecialized<439>(a);
		case 85:
			return RadicalInverseSpecialized<443>(a);
		case 86:
			return RadicalInverseSpecialized<449>(a);
		case 87:
			return RadicalInverseSpecialized<457>(a);
		case 88:
			return RadicalInverseSpecialized<461>(a);
		case 89:
			return RadicalInverseSpecialized<463>(a);
		case 90:
			return RadicalInverseSpecialized<467>(a);
		case 91:
			return RadicalInverseSpecialized<479>(a);
		case 92:
			return RadicalInverseSpecialized<487>(a);
		case 93:
			return RadicalInverseSpecialized<491>(a);
		case 94:
			return RadicalInverseSpecialized<499>(a);
		case 95:
			return RadicalInverseSpecialized<503>(a);
		case 96:
			return RadicalInverseSpecialized<509>(a);
		case 97:
			return RadicalInverseSpecialized<521>(a);
		case 98:
			return RadicalInverseSpecialized<523>(a);
		case 99:
			return RadicalInverseSpecialized<541>(a);
		case 100:
			return RadicalInverseSpecialized<547>(a);
		default:
			Log::Error("Base {} is >= 1024, the limit of RadicalInverseSpecialized", baseIndex);
			return 0;
		}
	}

	Float ScrambledRadicalInverse(int baseIndex, uint64_t a, const uint16_t* perm) 
	{
		switch (baseIndex) 
		{
		case 0:
			return ScrambledRadicalInverseSpecialized<2>(perm, a);
		case 1:
			return ScrambledRadicalInverseSpecialized<3>(perm, a);
		case 2:
			return ScrambledRadicalInverseSpecialized<5>(perm, a);
		case 3:
			return ScrambledRadicalInverseSpecialized<7>(perm, a);
		case 4:
			return ScrambledRadicalInverseSpecialized<11>(perm, a);
		case 5:
			return ScrambledRadicalInverseSpecialized<13>(perm, a);
		case 6:
			return ScrambledRadicalInverseSpecialized<17>(perm, a);
		case 7:
			return ScrambledRadicalInverseSpecialized<19>(perm, a);
		case 8:
			return ScrambledRadicalInverseSpecialized<23>(perm, a);
		case 9:
			return ScrambledRadicalInverseSpecialized<29>(perm, a);
		case 10:
			return ScrambledRadicalInverseSpecialized<31>(perm, a);
		case 11:
			return ScrambledRadicalInverseSpecialized<37>(perm, a);
		case 12:
			return ScrambledRadicalInverseSpecialized<41>(perm, a);
		case 13:
			return ScrambledRadicalInverseSpecialized<43>(perm, a);
		case 14:
			return ScrambledRadicalInverseSpecialized<47>(perm, a);
		case 15:
			return ScrambledRadicalInverseSpecialized<53>(perm, a);
		case 16:
			return ScrambledRadicalInverseSpecialized<59>(perm, a);
		case 17:
			return ScrambledRadicalInverseSpecialized<61>(perm, a);
		case 18:
			return ScrambledRadicalInverseSpecialized<67>(perm, a);
		case 19:
			return ScrambledRadicalInverseSpecialized<71>(perm, a);
		case 20:
			return ScrambledRadicalInverseSpecialized<73>(perm, a);
		case 21:
			return ScrambledRadicalInverseSpecialized<79>(perm, a);
		case 22:
			return ScrambledRadicalInverseSpecialized<83>(perm, a);
		case 23:
			return ScrambledRadicalInverseSpecialized<89>(perm, a);
		case 24:
			return ScrambledRadicalInverseSpecialized<97>(perm, a);
		case 25:
			return ScrambledRadicalInverseSpecialized<101>(perm, a);
		case 26:
			return ScrambledRadicalInverseSpecialized<103>(perm, a);
		case 27:
			return ScrambledRadicalInverseSpecialized<107>(perm, a);
		case 28:
			return ScrambledRadicalInverseSpecialized<109>(perm, a);
		case 29:
			return ScrambledRadicalInverseSpecialized<113>(perm, a);
		case 30:
			return ScrambledRadicalInverseSpecialized<127>(perm, a);
		case 31:
			return ScrambledRadicalInverseSpecialized<131>(perm, a);
		case 32:
			return ScrambledRadicalInverseSpecialized<137>(perm, a);
		case 33:
			return ScrambledRadicalInverseSpecialized<139>(perm, a);
		case 34:
			return ScrambledRadicalInverseSpecialized<149>(perm, a);
		case 35:
			return ScrambledRadicalInverseSpecialized<151>(perm, a);
		case 36:
			return ScrambledRadicalInverseSpecialized<157>(perm, a);
		case 37:
			return ScrambledRadicalInverseSpecialized<163>(perm, a);
		case 38:
			return ScrambledRadicalInverseSpecialized<167>(perm, a);
		case 39:
			return ScrambledRadicalInverseSpecialized<173>(perm, a);
		case 40:
			return ScrambledRadicalInverseSpecialized<179>(perm, a);
		case 41:
			return ScrambledRadicalInverseSpecialized<181>(perm, a);
		case 42:
			return ScrambledRadicalInverseSpecialized<191>(perm, a);
		case 43:
			return ScrambledRadicalInverseSpecialized<193>(perm, a);
		case 44:
			return ScrambledRadicalInverseSpecialized<197>(perm, a);
		case 45:
			return ScrambledRadicalInverseSpecialized<199>(perm, a);
		case 46:
			return ScrambledRadicalInverseSpecialized<211>(perm, a);
		case 47:
			return ScrambledRadicalInverseSpecialized<223>(perm, a);
		case 48:
			return ScrambledRadicalInverseSpecialized<227>(perm, a);
		case 49:
			return ScrambledRadicalInverseSpecialized<229>(perm, a);
		case 50:
			return ScrambledRadicalInverseSpecialized<233>(perm, a);
		case 51:
			return ScrambledRadicalInverseSpecialized<239>(perm, a);
		case 52:
			return ScrambledRadicalInverseSpecialized<241>(perm, a);
		case 53:
			return ScrambledRadicalInverseSpecialized<251>(perm, a);
		case 54:
			return ScrambledRadicalInverseSpecialized<257>(perm, a);
		case 55:
			return ScrambledRadicalInverseSpecialized<263>(perm, a);
		case 56:
			return ScrambledRadicalInverseSpecialized<269>(perm, a);
		case 57:
			return ScrambledRadicalInverseSpecialized<271>(perm, a);
		case 58:
			return ScrambledRadicalInverseSpecialized<277>(perm, a);
		case 59:
			return ScrambledRadicalInverseSpecialized<281>(perm, a);
		case 60:
			return ScrambledRadicalInverseSpecialized<283>(perm, a);
		case 61:
			return ScrambledRadicalInverseSpecialized<293>(perm, a);
		case 62:
			return ScrambledRadicalInverseSpecialized<307>(perm, a);
		case 63:
			return ScrambledRadicalInverseSpecialized<311>(perm, a);
		case 64:
			return ScrambledRadicalInverseSpecialized<313>(perm, a);
		case 65:
			return ScrambledRadicalInverseSpecialized<317>(perm, a);
		case 66:
			return ScrambledRadicalInverseSpecialized<331>(perm, a);
		case 67:
			return ScrambledRadicalInverseSpecialized<337>(perm, a);
		case 68:
			return ScrambledRadicalInverseSpecialized<347>(perm, a);
		case 69:
			return ScrambledRadicalInverseSpecialized<349>(perm, a);
		case 70:
			return ScrambledRadicalInverseSpecialized<353>(perm, a);
		case 71:
			return ScrambledRadicalInverseSpecialized<359>(perm, a);
		case 72:
			return ScrambledRadicalInverseSpecialized<367>(perm, a);
		case 73:
			return ScrambledRadicalInverseSpecialized<373>(perm, a);
		case 74:
			return ScrambledRadicalInverseSpecialized<379>(perm, a);
		case 75:
			return ScrambledRadicalInverseSpecialized<383>(perm, a);
		case 76:
			return ScrambledRadicalInverseSpecialized<389>(perm, a);
		case 77:
			return ScrambledRadicalInverseSpecialized<397>(perm, a);
		case 78:
			return ScrambledRadicalInverseSpecialized<401>(perm, a);
		case 79:
			return ScrambledRadicalInverseSpecialized<409>(perm, a);
		case 80:
			return ScrambledRadicalInverseSpecialized<419>(perm, a);
		case 81:
			return ScrambledRadicalInverseSpecialized<421>(perm, a);
		case 82:
			return ScrambledRadicalInverseSpecialized<431>(perm, a);
		case 83:
			return ScrambledRadicalInverseSpecialized<433>(perm, a);
		case 84:
			return ScrambledRadicalInverseSpecialized<439>(perm, a);
		case 85:
			return ScrambledRadicalInverseSpecialized<443>(perm, a);
		case 86:
			return ScrambledRadicalInverseSpecialized<449>(perm, a);
		case 87:
			return ScrambledRadicalInverseSpecialized<457>(perm, a);
		case 88:
			return ScrambledRadicalInverseSpecialized<461>(perm, a);
		case 89:
			return ScrambledRadicalInverseSpecialized<463>(perm, a);
		case 90:
			return ScrambledRadicalInverseSpecialized<467>(perm, a);
		case 91:
			return ScrambledRadicalInverseSpecialized<479>(perm, a);
		case 92:
			return ScrambledRadicalInverseSpecialized<487>(perm, a);
		case 93:
			return ScrambledRadicalInverseSpecialized<491>(perm, a);
		case 94:
			return ScrambledRadicalInverseSpecialized<499>(perm, a);
		case 95:
			return ScrambledRadicalInverseSpecialized<503>(perm, a);
		case 96:
			return ScrambledRadicalInverseSpecialized<509>(perm, a);
		case 97:
			return ScrambledRadicalInverseSpecialized<521>(perm, a);
		case 98:
			return ScrambledRadicalInverseSpecialized<523>(perm, a);
		case 99:
			return ScrambledRadicalInverseSpecialized<541>(perm, a);
		case 100:
			return ScrambledRadicalInverseSpecialized<547>(perm, a);
		case 101:
			return ScrambledRadicalInverseSpecialized<557>(perm, a);
		case 102:
			return ScrambledRadicalInverseSpecialized<563>(perm, a);
		case 103:
			return ScrambledRadicalInverseSpecialized<569>(perm, a);
		case 104:
			return ScrambledRadicalInverseSpecialized<571>(perm, a);
		case 105:
			return ScrambledRadicalInverseSpecialized<577>(perm, a);
		case 106:
			return ScrambledRadicalInverseSpecialized<587>(perm, a);
		case 107:
			return ScrambledRadicalInverseSpecialized<593>(perm, a);
		case 108:
			return ScrambledRadicalInverseSpecialized<599>(perm, a);
		case 109:
			return ScrambledRadicalInverseSpecialized<601>(perm, a);
		case 110:
			return ScrambledRadicalInverseSpecialized<607>(perm, a);
		case 111:
			return ScrambledRadicalInverseSpecialized<613>(perm, a);
		case 112:
			return ScrambledRadicalInverseSpecialized<617>(perm, a);
		case 113:
			return ScrambledRadicalInverseSpecialized<619>(perm, a);
		case 114:
			return ScrambledRadicalInverseSpecialized<631>(perm, a);
		case 115:
			return ScrambledRadicalInverseSpecialized<641>(perm, a);
		case 116:
			return ScrambledRadicalInverseSpecialized<643>(perm, a);
		case 117:
			return ScrambledRadicalInverseSpecialized<647>(perm, a);
		case 118:
			return ScrambledRadicalInverseSpecialized<653>(perm, a);
		case 119:
			return ScrambledRadicalInverseSpecialized<659>(perm, a);
		case 120:
			return ScrambledRadicalInverseSpecialized<661>(perm, a);
		case 121:
			return ScrambledRadicalInverseSpecialized<673>(perm, a);
		case 122:
			return ScrambledRadicalInverseSpecialized<677>(perm, a);
		case 123:
			return ScrambledRadicalInverseSpecialized<683>(perm, a);
		case 124:
			return ScrambledRadicalInverseSpecialized<691>(perm, a);
		case 125:
			return ScrambledRadicalInverseSpecialized<701>(perm, a);
		case 126:
			return ScrambledRadicalInverseSpecialized<709>(perm, a);
		case 127:
			return ScrambledRadicalInverseSpecialized<719>(perm, a);
		case 128:
			return ScrambledRadicalInverseSpecialized<727>(perm, a);
		case 129:
			return ScrambledRadicalInverseSpecialized<733>(perm, a);
		case 130:
			return ScrambledRadicalInverseSpecialized<739>(perm, a);
		case 131:
			return ScrambledRadicalInverseSpecialized<743>(perm, a);
		case 132:
			return ScrambledRadicalInverseSpecialized<751>(perm, a);
		case 133:
			return ScrambledRadicalInverseSpecialized<757>(perm, a);
		case 134:
			return ScrambledRadicalInverseSpecialized<761>(perm, a);
		case 135:
			return ScrambledRadicalInverseSpecialized<769>(perm, a);
		case 136:
			return ScrambledRadicalInverseSpecialized<773>(perm, a);
		case 137:
			return ScrambledRadicalInverseSpecialized<787>(perm, a);
		case 138:
			return ScrambledRadicalInverseSpecialized<797>(perm, a);
		case 139:
			return ScrambledRadicalInverseSpecialized<809>(perm, a);
		case 140:
			return ScrambledRadicalInverseSpecialized<811>(perm, a);
		case 141:
			return ScrambledRadicalInverseSpecialized<821>(perm, a);
		case 142:
			return ScrambledRadicalInverseSpecialized<823>(perm, a);
		case 143:
			return ScrambledRadicalInverseSpecialized<827>(perm, a);
		case 144:
			return ScrambledRadicalInverseSpecialized<829>(perm, a);
		case 145:
			return ScrambledRadicalInverseSpecialized<839>(perm, a);
		case 146:
			return ScrambledRadicalInverseSpecialized<853>(perm, a);
		case 147:
			return ScrambledRadicalInverseSpecialized<857>(perm, a);
		case 148:
			return ScrambledRadicalInverseSpecialized<859>(perm, a);
		case 149:
			return ScrambledRadicalInverseSpecialized<863>(perm, a);
		case 150:
			return ScrambledRadicalInverseSpecialized<877>(perm, a);
		case 151:
			return ScrambledRadicalInverseSpecialized<881>(perm, a);
		case 152:
			return ScrambledRadicalInverseSpecialized<883>(perm, a);
		case 153:
			return ScrambledRadicalInverseSpecialized<887>(perm, a);
		case 154:
			return ScrambledRadicalInverseSpecialized<907>(perm, a);
		case 155:
			return ScrambledRadicalInverseSpecialized<911>(perm, a);
		case 156:
			return ScrambledRadicalInverseSpecialized<919>(perm, a);
		case 157:
			return ScrambledRadicalInverseSpecialized<929>(perm, a);
		case 158:
			return ScrambledRadicalInverseSpecialized<937>(perm, a);
		case 159:
			return ScrambledRadicalInverseSpecialized<941>(perm, a);
		case 160:
			return ScrambledRadicalInverseSpecialized<947>(perm, a);
		case 161:
			return ScrambledRadicalInverseSpecialized<953>(perm, a);
		case 162:
			return ScrambledRadicalInverseSpecialized<967>(perm, a);
		case 163:
			return ScrambledRadicalInverseSpecialized<971>(perm, a);
		case 164:
			return ScrambledRadicalInverseSpecialized<977>(perm, a);
		case 165:
			return ScrambledRadicalInverseSpecialized<983>(perm, a);
		case 166:
			return ScrambledRadicalInverseSpecialized<991>(perm, a);
		case 167:
			return ScrambledRadicalInverseSpecialized<997>(perm, a);
		case 168:
			return ScrambledRadicalInverseSpecialized<1009>(perm, a);
		case 169:
			return ScrambledRadicalInverseSpecialized<1013>(perm, a);
		case 170:
			return ScrambledRadicalInverseSpecialized<1019>(perm, a);
		case 171:
			return ScrambledRadicalInverseSpecialized<1021>(perm, a);
		case 172:
			return ScrambledRadicalInverseSpecialized<1031>(perm, a);
		case 173:
			return ScrambledRadicalInverseSpecialized<1033>(perm, a);
		case 174:
			return ScrambledRadicalInverseSpecialized<1039>(perm, a);
		case 175:
			return ScrambledRadicalInverseSpecialized<1049>(perm, a);
		case 176:
			return ScrambledRadicalInverseSpecialized<1051>(perm, a);
		case 177:
			return ScrambledRadicalInverseSpecialized<1061>(perm, a);
		case 178:
			return ScrambledRadicalInverseSpecialized<1063>(perm, a);
		case 179:
			return ScrambledRadicalInverseSpecialized<1069>(perm, a);
		case 180:
			return ScrambledRadicalInverseSpecialized<1087>(perm, a);
		case 181:
			return ScrambledRadicalInverseSpecialized<1091>(perm, a);
		case 182:
			return ScrambledRadicalInverseSpecialized<1093>(perm, a);
		case 183:
			return ScrambledRadicalInverseSpecialized<1097>(perm, a);
		case 184:
			return ScrambledRadicalInverseSpecialized<1103>(perm, a);
		case 185:
			return ScrambledRadicalInverseSpecialized<1109>(perm, a);
		case 186:
			return ScrambledRadicalInverseSpecialized<1117>(perm, a);
		case 187:
			return ScrambledRadicalInverseSpecialized<1123>(perm, a);
		case 188:
			return ScrambledRadicalInverseSpecialized<1129>(perm, a);
		case 189:
			return ScrambledRadicalInverseSpecialized<1151>(perm, a);
		case 190:
			return ScrambledRadicalInverseSpecialized<1153>(perm, a);
		case 191:
			return ScrambledRadicalInverseSpecialized<1163>(perm, a);
		case 192:
			return ScrambledRadicalInverseSpecialized<1171>(perm, a);
		case 193:
			return ScrambledRadicalInverseSpecialized<1181>(perm, a);
		case 194:
			return ScrambledRadicalInverseSpecialized<1187>(perm, a);
		case 195:
			return ScrambledRadicalInverseSpecialized<1193>(perm, a);
		case 196:
			return ScrambledRadicalInverseSpecialized<1201>(perm, a);
		case 197:
			return ScrambledRadicalInverseSpecialized<1213>(perm, a);
		case 198:
			return ScrambledRadicalInverseSpecialized<1217>(perm, a);
		case 199:
			return ScrambledRadicalInverseSpecialized<1223>(perm, a);
		case 200:
			return ScrambledRadicalInverseSpecialized<1229>(perm, a);
		default:
			Log::Error("Base {} is >= 1024, the limit of ScrambledRadicalInverse",
				baseIndex);
		}

		return 0;
	}

	std::vector<uint16_t> ComputeRadicalInversePermutations(RNG& rng) 
	{
		std::vector<uint16_t> perms;
		// Allocate space in _perms_ for radical inverse permutations
		//permArraySize等于PrimeSums[PrimeTableSize - 1] + Primes[PrimeTableSize - 1]
		int permArraySize = 0;
		for (int i = 0; i < PrimeTableSize; ++i) 
			permArraySize += Primes[i];
		perms.resize(permArraySize);
		uint16_t* p = &perms[0];
		for (int i = 0; i < PrimeTableSize; ++i) 
		{
			// Generate random permutation for $i$th prime base
			for (int j = 0; j < Primes[i]; ++j) 
				p[j] = j;
			//对base的digit进行重新排序，排序后的base存在perms里
			Shuffle(p, Primes[i], 1, rng);
			p += Primes[i];
		}
		return perms;
	}
}
