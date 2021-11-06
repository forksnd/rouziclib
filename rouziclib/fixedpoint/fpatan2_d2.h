const uint32_t lutsp = 7, prec = 32;
static const int32_t fpatan2_lut_off[] = {511331160, 680887518, 169556351, 514001106, 686227383, 172226283, 516586314, 691479831, 174894189, 519084245, 696636803, 177555842, 521492441, 701690009, 180206688, 523808540, 706630949, 182841834, 526030287, 711450928, 185456031, 528155553, 716141075, 188043661, 530182349, 720692365, 190598726, 532108842, 725095652, 193114835, 533933373, 729341693, 195585193, 535654475, 733421182, 198002591, 537270886, 737324792, 200359399, 538781575, 741043216, 202647558, 540185751, 744567210, 204858575, 541482888, 747887645, 206983521, 542672736, 750995563, 209013033, 543755342, 753882235, 210937313, 544731066, 756539226, 212746141, 545600593, 758958458, 214428876, 546364953, 761132287, 215974479, 547025524, 763053577, 217371525, 547584053, 764715778, 218608229, 548042658, 766113006, 219672474, 548403835, 767240134, 220551844, 548670465, 768092874, 221233664, 548845810, 768667864, 221705045, 548933514, 768962761, 221952938, 548937595, 768976325, 221964190, 548862435, 768708506, 221725610, 548712763, 768160531, 221224042, 548493643, 767334979, 220446446, 548210443, 766235856, 219379977, 547868814, 764868665, 218012086, 547474652, 763240464, 216330611, 547034064, 761359914, 214323886, 546553327, 759237314, 211980848, 546038839, 756884632, 209291153, 545497073, 754315508, 206245291, 544934517, 751545248, 202834706, 544357625, 748590803, 199051921, 543772751, 745470721, 194890654, 543186092, 742205087, 190345940, 542603622, 738815433, 185414249, 542031033, 735324640, 180093592, 541473667, 731756802, 174383634, 540936465, 728137085, 168285784, 540423902, 724491549, 161803282, 539939934, 720846966, 154941277, 539487957, 717230607, 147706880, 539070758, 713670015, 140109211, 538690480, 710192773, 132159428, 538348602, 706826245, 123870729, 538045913, 703597319, 115258342, 537782513, 700532142, 106339495, 537557805, 697655855, 97133353, 537370518, 694992323, 87660951, 537218725, 692563885, 77945088, 537099876, 690391101, 68010211, 537010847, 688492522, 57882276, 536947994, 686884479, 47588589, 536907213, 685580888, 37157626, 536884013, 684593090, 26618850, 536873596, 683929710, 16002499, 536870939, 683596558, 5339376, 536870885, 683596558, -5339376, 536868228, 683929710, -16002499, 536857811, 684593090, -26618850, 536834611, 685580888, -37157626, 536793830, 686884479, -47588589, 536730977, 688492522, -57882276, 536641948, 690391101, -68010211, 536523099, 692563885, -77945088, 536371306, 694992323, -87660951, 536184019, 697655855, -97133353, 535959311, 700532142, -106339495, 535695911, 703597319, -115258342, 535393222, 706826245, -123870729, 535051344, 710192773, -132159428, 534671066, 713670015, -140109211, 534253867, 717230606, -147706880, 533801890, 720846966, -154941277, 533317922, 724491549, -161803282, 532805359, 728137085, -168285784, 532268157, 731756802, -174383634, 531710791, 735324640, -180093592, 531138202, 738815433, -185414249, 530555732, 742205087, -190345940, 529969073, 745470721, -194890654, 529384199, 748590803, -199051921, 528807307, 751545248, -202834706, 528244751, 754315508, -206245291, 527702985, 756884632, -209291153, 527188497, 759237314, -211980848, 526707760, 761359914, -214323886, 526267172, 763240464, -216330611, 525873010, 764868665, -218012086, 525531381, 766235856, -219379977, 525248181, 767334979, -220446446, 525029061, 768160531, -221224042, 524879389, 768708506, -221725610, 524804229, 768976325, -221964190, 524808310, 768962761, -221952938, 524896014, 768667864, -221705045, 525071359, 768092874, -221233664, 525337989, 767240134, -220551844, 525699166, 766113006, -219672474, 526157771, 764715778, -218608229, 526716300, 763053577, -217371525, 527376871, 761132287, -215974479, 528141231, 758958458, -214428876, 529010758, 756539226, -212746141, 529986482, 753882235, -210937313, 531069088, 750995563, -209013033, 532258936, 747887645, -206983521, 533556073, 744567210, -204858575, 534960249, 741043216, -202647558, 536470938, 737324792, -200359399, 538087349, 733421182, -198002591, 539808451, 729341693, -195585193, 541632982, 725095652, -193114835, 543559475, 720692365, -190598726, 545586271, 716141075, -188043661, 547711537, 711450928, -185456031, 549933284, 706630949, -182841834, 552249383, 701690009, -180206688, 554657579, 696636803, -177555842, 557155510, 691479831, -174894189, 559740718, 686227383, -172226283, 559740718, 686227383, -172226283};
static const int32_t *fpatan2_lut = &fpatan2_lut_off[195];