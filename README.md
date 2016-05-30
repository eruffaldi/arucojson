# arucojson
Command-line Aruco Marker to JSON computation. This small package iuse

# Tools

## aruco2json

Requires a calibration file (YAML OpenCV), a marker size (m), input file and a flag to tell if the image is already undistorted. This will produce a json containing all the required information.
It can also export or visualize the markers overlaid

- camera information
- per-marker data comprising the area of the marker visible in pixels, corners, OpenGL Modelview matrix

Syntax:

 aruco2json calibrationfile size imagefile alreadyundist [-|outfilename]

Example Output:

	{
	  "K": [
	    [
	      532.85,
	      0,
	      316.88
	    ],
	    [
	      0,
	      532.45,
	      241.63
	    ],
	    [
	      0,
	      0,
	      1
	    ]
	  ],
	  "dist": [
	    [
	      0,
	      0,
	      0,
	      0,
	      0
	    ]
	  ],
	  "glprojection": [
	    1.6651561737061,
	    -0,
	    0,
	    0,
	    0,
	    -2.6622500610352,
	    0,
	    0,
	    0.0097499847412109,
	    0.20815002441406,
	    -1.002002002002,
	    -1,
	    0,
	    -0,
	    -0.2002002002002,
	    0
	  ],
	  "imagesize": [
	    640,
	    400
	  ],
	  "markers": [
	    {
	      "Tvec": [
	        0.012566397897899,
	        0.033242721110582,
	        0.48332592844963
	      ],
	      "areapx": 11068.0390625,
	      "areau": 0.043234527111053,
	      "center": [
	        330.40185546875,
	        280.71496582031
	      ],
	      "error": 1.7395777717263,
	      "glmodelview": [
	        -0.053420249372721,
	        0.87593740224838,
	        0.47945794463158,
	        0,
	        0.99592757225037,
	        0.081656329333782,
	        -0.038216348737478,
	        0,
	        0.072625905275345,
	        -0.4754638671875,
	        0.87673234939575,
	        0,
	        0.012566393241286,
	        0.033242717385292,
	        -0.48332592844963,
	        1
	      ],
	      "id": 26,
	      "points": [
	        [
	          281.42459106445,
	          226.32289123535
	        ],
	        [
	          383.91390991211,
	          234.3843536377
	        ],
	        [
	          386.38558959961,
	          335.71697998047
	        ],
	        [
	          269.88327026367,
	          326.43569946289
	        ],
	        [
	          330.40185546875,
	          280.71496582031
	        ]
	      ],
	      "pose": [
	        [
	          -0.053420305252075,
	          0.99592757225037,
	          0.072625935077667,
	          0.012566397897899
	        ],
	        [
	          0.87593746185303,
	          0.081656388938427,
	          -0.47546374797821,
	          0.033242721110582
	        ],
	        [
	          -0.47945785522461,
	          0.038216356188059,
	          -0.8767324090004,
	          0.48332592844963
	        ],
	        [
	          0,
	          0,
	          0,
	          1
	        ]
	      ]
	    }
	  ],
	  "markersize": 0.10000000149012,
	  "yaxisup": false
	}

## undistort

Simple tool for undistorting an image: calibrationfile input output

## overlay

Simple tool for overlaying generic marker information over image. Command contains YAML 

Syntax:

 overlay calibrationfile.yaml command.yaml imagefile dist [outfile|-]

Example:
	%YAML:1.0
	markerid1: 1
	markerid2: 1
	markerpose1: !!opencv-matrix
	  cols: 4
	  data: [-0.0534203052520752, 0.9959275722503662, 0.07262593507766724, 0.01256639789789915,
	    0.8759374618530273, 0.08165638893842697, -0.4754637479782104, 0.03324272111058235,
	    -0.4794578552246094, 0.03821635618805885, -0.8767324090003967, 0.4833259284496307,
	    0.0, 0.0, 0.0, 1.0]
	  dt: d
	  rows: 4
	markerpose2: !!opencv-matrix
	  cols: 4
	  data: [-0.0534203052520752, 0.9959275722503662, 0.07262593507766724, 0.03640603199193842,
	    0.8759374618530273, 0.08165638893842697, -0.4754637479782104, -0.12026646523210135,
	    -0.4794578552246094, 0.03821635618805885, -0.8767324090003967, 0.568550824938581,
	    0.0, 0.0, 0.0, 1.0]
	  dt: d
	  rows: 4
	markersize1: 0.1000000014901161
	markersize2: 0.03
	mode1: 3
	mode2: 3


# Building

Use cmakego for simple cmake dependencies specification (https://github.com/eruffaldi/cmakego)
