const float offset = 131076.f, limit = 4.f;
static const float fasterfr_lut[] = {2.86872272e-07f, 6.97956161e-08f, 3.54511589e-07f, 8.68105727e-08f, 4.50265667e-07f, 1.110833e-07f, 5.70737287e-07f, 1.41865501e-07f, 7.21991978e-07f, 1.80824259e-07f, 9.11500483e-07f, 2.30032109e-07f, 1.14844321e-06f, 2.92060014e-07f, 1.44407556e-06f, 3.70090224e-07f, 1.81216508e-06f, 4.68052961e-07f, 2.26951319e-06f, 5.90791519e-07f, 2.83657612e-06f, 7.44261202e-07f, 3.53820185e-06f, 9.3576841e-07f, 4.40450241e-06f, 1.17425725e-06f, 5.47188327e-06f, 1.47065226e-06f, 6.78425493e-06f, 1.83826708e-06f, 8.39445462e-06f, 2.29329064e-06f, 1.03659099e-05f, 2.85536385e-06f, 1.27745792e-05f, 3.54826207e-06f, 1.57112094e-05f, 4.4007003e-06f, 1.92839533e-05f, 5.44728086e-06f, 2.3621396e-05f, 6.72960565e-06f, 2.88760425e-05f, 8.29757783e-06f, 3.52283242e-05f, 1.02109212e-05f, 4.28911872e-05f, 1.25409486e-05f, 5.21153289e-05f, 1.53726141e-05f, 6.31951554e-05f, 1.88068877e-05f, 7.64755348e-05f, 2.29634963e-05f, 9.23594272e-05f, 2.79840754e-05f, 0.000111316473f, 3.40357861e-05f, 0.000133892629f, 4.13154494e-05f, 0.000160720927f, 5.00542604e-05f, 0.000192533463f, 6.05231451e-05f, 0.000230174678f, 7.30388287e-05f, 0.000274616024f, 8.79706869e-05f, 0.000326972097f, 0.000105748457f, 0.000388518281f, 0.000126870884f, 0.000460709992f, 0.000151915387f, 0.000545203523f, 0.000181548823f, 0.000643878557f, 0.000216539426f, 0.000758862308f, 0.000257770014f, 0.000892555294f, 0.000306252529f, 0.00104765867f, 0.000363143975f, 0.00122720304f, 0.000429763843f, 0.00143457859f, 0.00050761305f, 0.00167356644f, 0.000598394446f, 0.00194837087f, 0.000704034922f, 0.00226365223f, 0.000826709107f, 0.0026245602f, 0.000968864657f, 0.00303676695f, 0.00113324909f, 0.0035064997f, 0.00132293807f, 0.0040405723f, 0.0015413651f, 0.00464641501f, 0.0017923524f, 0.00533210203f, 0.00208014282f, 0.00610637587f, 0.00240943257f, 0.00697866778f, 0.00278540451f, 0.00795911351f, 0.00321376149f, 0.00905856326f, 0.00370075967f, 0.0102885851f, 0.00425324103f, 0.0116614606f, 0.00487866479f, 0.0131901719f, 0.00558513707f, 0.0148883791f, 0.00638143815f, 0.0167703868f, 0.00727704675f, 0.0188510993f, 0.00828216041f, 0.0211459626f, 0.00940771132f, 0.023670894f, 0.0106653767f, 0.0264421963f, 0.0120675828f, 0.0294764585f, 0.0136275018f, 0.0327904405f, 0.0153590404f, 0.0364009429f, 0.0172768192f, 0.0403246616f, 0.0193961425f, 0.0445780265f, 0.0217329567f, 0.0491770266f, 0.0243037974f, 0.0541370207f, 0.0271257234f, 0.0594725359f, 0.0302162382f, 0.065197055f, 0.0335931967f, 0.0713227941f, 0.0372746988f, 0.0778604738f, 0.0412789674f, 0.0848190852f, 0.0456242122f, 0.0922056551f, 0.0503284789f, 0.100025012f, 0.0554094836f, 0.108279555f, 0.060884434f, 0.11696904f, 0.066769837f, 0.126090362f, 0.0730812943f, 0.135637375f, 0.0798332881f, 0.145600715f, 0.0870389565f, 0.155967658f, 0.0947098626f, 0.166722006f, 0.102855758f, 0.177844002f, 0.111484344f, 0.189310291f, 0.120601034f, 0.201093911f, 0.130208714f, 0.213164339f, 0.140307518f, 0.225487577f, 0.150894603f, 0.238026286f, 0.161963943f, 0.250739977f, 0.17350614f, 0.263585249f, 0.18550825f, 0.276516076f, 0.197953635f, 0.289484147f, 0.210821842f, 0.302439256f, 0.224088516f, 0.315329734f, 0.237725335f, 0.328102926f, 0.251699996f, 0.340705702f, 0.265976227f, 0.353085004f, 0.280513848f, 0.365188417f, 0.295268871f, 0.376964759f, 0.310193642f, 0.38836468f, 0.325237035f, 0.39934127f, 0.34034468f, 0.409850651f, 0.355459243f, 0.419852566f, 0.370520745f, 0.429310935f, 0.385466924f, 0.438194377f, 0.400233632f, 0.446476702f, 0.41475527f, 0.454137335f, 0.428965257f, 0.461161693f, 0.442796518f, 0.467541493f, 0.456182002f, 0.473274983f, 0.469055215f, 0.478367101f, 0.481350762f, 0.482829543f, 0.493004893f, 0.486680754f, 0.503956056f, 0.489945823f, 0.514145429f, 0.492656297f, 0.52351745f, 0.494849897f, 0.532020316f, 0.496570162f, 0.539606465f, 0.497866004f, 0.546233012f, 0.498791187f, 0.551862158f, 0.499403743f, 0.556461546f, 0.499765319f, 0.560004573f, 0.499940481f, 0.562470645f, 0.499995967f, 0.563845373f, 0.49999991f, 0.564120722f, 0.500021051f, 0.563295079f, 0.500127927f, 0.561373275f, 0.500388072f, 0.558366536f, 0.500867233f, 0.554292372f, 0.501628607f, 0.549174414f, 0.502732117f, 0.543042175f, 0.50423374f, 0.535930775f, 0.506184884f, 0.527880605f, 0.508631832f, 0.51893694f, 0.511615267f, 0.509149521f, 0.515169862f, 0.498572093f, 0.519323964f, 0.487261909f, 0.524099359f, 0.475279226f, 0.529511124f, 0.462686762f, 0.535567572f, 0.449549157f, 0.542270283f, 0.435932426f, 0.549614214f, 0.421903407f, 0.557587898f, 0.407529229f, 0.566173714f, 0.392876783f, 0.575348229f, 0.37801222f, 0.585082599f, 0.363000469f, 0.595343031f, 0.347904787f, 0.606091288f, 0.332786343f, 0.617285232f, 0.317703836f, 0.628879394f, 0.302713154f, 0.640825568f, 0.287867079f, 0.653073415f, 0.273215026f, 0.665571059f, 0.258802836f, 0.678265693f, 0.24467261f, 0.691104152f, 0.230862583f, 0.704033478f, 0.217407048f, 0.717001446f, 0.204336316f, 0.729957062f, 0.191676721f, 0.742851016f, 0.179450658f, 0.755636094f, 0.167676659f, 0.768267543f, 0.156369497f, 0.780703386f, 0.145540325f, 0.792904681f, 0.135196831f, 0.80483574f, 0.125343425f, 0.816464289f, 0.115981434f, 0.827761577f, 0.10710932f, 0.838702443f, 0.0987229044f, 0.849265334f, 0.0908156001f, 0.859432281f, 0.0833786506f, 0.869188835f, 0.0764013688f, 0.878523968f, 0.0698713749f, 0.887429945f, 0.0637748301f, 0.895902164f, 0.0580966652f, 0.903938975f, 0.0528207994f, 0.911541481f, 0.0479303508f, 0.918713325f, 0.0434078342f, 0.925460464f, 0.0392353472f, 0.931790936f, 0.0353947422f, 0.937714626f, 0.0318677851f, 0.943243032f, 0.0286362983f, 0.948389031f, 0.0256822902f, 0.953166655f, 0.022988069f, 0.957590873f, 0.0205363428f, 0.961677385f, 0.0183103056f, 0.965442421f, 0.0162937088f, 0.968902566f, 0.0144709213f, 0.972074588f, 0.0128269761f, 0.974975283f, 0.0113476063f, 0.977621341f, 0.0100192706f, 0.980029223f, 0.00882916893f, 0.982215048f, 0.00776524956f, 0.98419451f, 0.00681620872f, 0.985982791f, 0.00597148322f, 0.987594501f, 0.0052212372f, 0.989043629f, 0.00455634397f, 0.990343498f, 0.00396836357f, 0.991506743f, 0.00344951691f, 0.99254529f, 0.00299265721f, 0.993470347f, 0.00259123942f, 0.994292408f, 0.00223928815f, 0.995021253f, 0.00193136472f, 0.995665967f, 0.00166253378f, 0.996234956f, 0.00142832992f, 0.996735969f, 0.00122472466f, 0.997176125f, 0.00104809406f, 0.997561939f, 0.000895187344f, 0.997899358f, 0.000763096528f, 0.998193789f, 0.00064922748f, 0.998450131f, 0.000551272314f, 0.998672815f, 0.000467183335f, 0.998865827f, 0.000395148534f, 0.999032751f, 0.00033356869f, 0.999176791f, 0.000281036065f, 0.999300811f, 0.000236314682f, 0.999407359f, 0.000198322151f, 0.999498693f, 0.000166112998f, 0.999576815f, 0.000138863428f, 0.99964349f, 0.000115857464f, 0.999700271f, 9.64743868e-05f, 0.999748521f, 8.01773886e-05f, 0.999789432f, 6.65033709e-05f, 0.999824046f, 5.50537978e-05f, 0.999853268f, 4.54865274e-05f, 0.999877885f, 3.75085392e-05f, 0.999898578f, 3.08694856e-05f, 0.999915936f, 2.53559859e-05f, 0.999930463f, 2.07866014e-05f, 0.999942597f, 1.70074163e-05f, 0.999952709f, 1.38881716e-05f, 0.999961119f, 1.13188857e-05f, 0.999968098f, 9.20691534e-06f, 0.999973877f, 7.47440265e-06f, 0.999978652f, 6.05606758e-06f, 0.999982589f, 4.89730095e-06f, 0.999985829f, 3.95252618e-06f, 0.99998849f, 3.18379131e-06f, 0.999990669f, 2.55956575e-06f, 0.999992452f, 2.05371358e-06f, 0.999993906f, 1.64461906e-06f, 0.999995089f, 1.31444566e-06f, 0.999996051f, 1.04850817e-06f, 0.999996831f, 8.34743214e-07f, 0.999997462f, 6.63263171e-07f, 0.999997972f, 5.25981723e-07f, 0.999998382f, 4.16300875e-07f, 0.999998712f, 3.28848628e-07f, 0.999998977f, 2.59260483e-07f, 0.999999189f, 2.03999314e-07f, 0.999999358f, 1.60203747e-07f, 0.999999493f, 1.25565072e-07f, 0.9999996f, 9.82237812e-08f, 0.999999686f, 7.66860038e-08f, 0.99999973f, 6.55132157e-08f};