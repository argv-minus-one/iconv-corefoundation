{
	"targets": [
		{
			"target_name": "iconv_corefoundation_native",
			"sources": [
				"src/iccf.cc",
				"src/StringEncoding.cc",
				"src/string-utils.cc",
				"src/transcode.cc"
			],
			"include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
			"dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
			"libraries": ["CoreFoundation.framework"],
			"cflags": ["-fvisibility=hidden", "-std=c++17"],
			"cflags!": ["-fno-exceptions"],
			"cflags_cc": ["-fvisibility=hidden", "-std=c++17"],
			"cflags_cc!": ["-fno-exceptions"],
			"xcode_settings": {
				"GCC_ENABLE_CPP_EXCEPTIONS": "YES",
				"GCC_SYMBOLS_PRIVATE_EXTERN": "YES",
				"CLANG_CXX_LIBRARY": "libc++",
				"MACOSX_DEPLOYMENT_TARGET": "10.7",
				"CLANG_CXX_LANGUAGE_STANDARD": "C++17",
				"OTHER_CFLAGS": [
					"-fvisibility=hidden",
					"-std=c++17"
				]
			},
		}
	]
}
