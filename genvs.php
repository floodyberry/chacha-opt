<?

function get_guid($name) {
	$hex = strtoupper(md5($name));
	return "{".substr($hex, 0, 8)."-".substr($hex, 8, 4)."-".substr($hex, 12, 4)."-".substr($hex, 16, 4)."-".substr($hex, 20, 12)."}";
}

function fecholn($f, $str) {
	fwrite($f, $str);
	fwrite($f, "\r\n");
}

function quote($str) {
	return "\"{$str}\"";
}

function fixslash($str) {
	return str_replace("/", "\\", $str);
}

function crawl(&$list, $dir, $grab, $recurse) {
	$dh = opendir($dir);
	if ($dh) {
		while (($file = readdir($dh)) !== false) {
			$path = $dir."/".$file;
			if (($file == ".") | $file == "..")
				continue;
			if (is_dir($path)) {
				if ($recurse)
					crawl($list, $path, $grab, $recurse);
			} else {
				foreach($grab as $pat) {
					if (preg_match($pat, $file)) {
						$list[] = fixslash($path);
						break;
					}
				}
			}
		}
		closedir($dh);
	}
}

abstract class gen_vs {
	protected $name;
	protected $builds;
	protected $projects;
	protected $sln;
	protected $project_dir;
	protected $files;

	public function gen_vs($name) {
		$this->name = strtolower($name);
		$this->projects = array();
		
		foreach(array("lib", "dll", "util") as $type) {
			$name = "{$this->name}_{$type}";
			$this->projects[$type] = array("name"=>$name, "guid"=>get_guid($name));
		}
	}
	
	public function build_files() {
		$this->files = array("driver"=>array(), "ext"=>array(), "util"=>array(), "shared"=>array(), "include"=>array());
		crawl($this->files["driver"], "driver", array("!\.c$!", "!\.h$!", "!\.inc$!"), false);
		crawl($this->files["driver"], "driver/x86", array("!\.c$!", "!\.S$!", "!\.h$!", "!\.inc$!"), false);
		crawl($this->files["ext"], "extensions", array("!\.c$!", "!\.S$!", "!\.inc$!", "!\.h$!"), true);
		crawl($this->files["include"], "include", array("!\.h$!"), false);
		crawl($this->files["shared"], "src", array("!^shared\.c$!"), false);
		crawl($this->files["util"], "src/util", array("!\.c$!", "!\.h$!"), true);
		crawl($this->files["util"], "src", array("!^util\.c$!"), false);

		$this->projects["lib"]["files"] = array("driver", "ext", "include"); 
		$this->projects["dll"]["files"] = array("driver", "ext", "include", "shared");
		$this->projects["util"]["files"] = array("driver", "ext", "include", "util"); 
	}

	public function write_file($name, $str) {
		$in = array("%%name%%", "%%NAME%%", "%%projectdir%%");
		$out = array($this->name, strtoupper($this->name), $this->project_dir);
		$name = str_replace($in, $out, $name);
		$str = str_replace($in, $out, $str);
		$f = fopen("{$name}", "w+");
		chmod("{$name}", 0755);
		fwrite($f, $str);
		fclose($f);
	}

	public abstract function make();
};

class vs2010 extends gen_vs {
	protected $fileinfo;

	public function vs2010($name) {
		parent::gen_vs($name);

		$this->project_dir = "vs2010";

		if (!file_exists($this->project_dir))
			mkdir($this->project_dir, 0755);

		$this->sln = "{$this->name}.sln";

		foreach($this->projects as $handle=>&$info)
			$info["vcxproj"] = "{$info['name']}.vcxproj";

		$this->builds = array(
			"Debug|x86-32bit"=>"Debug|Win32",
			"Debug|amd64"=>"Debug|x64",
			"Release|x86-32bit"=>"Release|Win32",
			"Release|amd64"=>"Release|x64"
		);
	}

	function make_sln() {
		$f = fopen("{$this->project_dir}/".$this->sln, "w+");
		fecholn($f, "Microsoft Visual Studio Solution File, Format Version 11.00");
		fecholn($f, "# Visual Studio 2010");

		foreach($this->projects as $handle=>$info) {
			fecholn($f, "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = ".quote($info["name"]).", ".quote($info["vcxproj"]).", ".quote($info["guid"]));
			fecholn($f, "EndProject");
		}

		fecholn($f, "Global");
			fecholn($f, "GlobalSection(SolutionConfigurationPlatforms) = preSolution");
			foreach($this->builds as $label=>$build)
				fecholn($f, "{$label} = {$label}");
			fecholn($f, "EndGlobalSection");

			fecholn($f, "GlobalSection(ProjectConfigurationPlatforms) = postSolution");
			foreach($this->projects as $handle=>$info) {
				foreach($this->builds as $label=>$build) {
					fecholn($f, "{$info['guid']}.{$label}.ActiveCfg = {$build}");
					fecholn($f, "{$info['guid']}.{$label}.Build.0 = {$build}");
				}
			}
			fecholn($f, "EndGlobalSection");

			fecholn($f, "GlobalSection(SolutionProperties) = preSolution");
			fecholn($f, "HideSolutionNode = FALSE");
			fecholn($f, "EndGlobalSection");
		fecholn($f, "EndGlobal");
		fclose($f);
	}
	
	public function make_vcxproj_filters() {
		foreach($this->projects as $handle=>$info) {
			$f = fopen("{$this->project_dir}/".$info["vcxproj"].".filters", "w+");

			fecholn($f, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
			fecholn($f, "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");
			fecholn($f, "<ItemGroup>");
				fecholn($f, "<Filter Include=\"Source\"></Filter>");

				$seen = array();
				foreach($info["files"] as $handle) {
					foreach($this->files[$handle] as $path) {
						while (1) {
							$chop_directory = preg_replace("!^(.*)\\\\.*$!", "$1", $path);
							if ($chop_directory === $path)
								break;
							$seen[$chop_directory] = 1;
							$path = $chop_directory;
						}
					}
				}

				foreach($seen as $basepath=>$dummy)
					fecholn($f, "<Filter Include=\"Source\\{$basepath}\"></Filter>");
			fecholn($f, "</ItemGroup>");

			foreach($info["files"] as $handle) {
				fecholn($f, "<ItemGroup>");
				foreach($this->files[$handle] as $path) {
					$type = $this->fileinfo[$path]["type"];
					$folder = $this->fileinfo[$path]["basepath"];
					fecholn($f, "<{$type} Include=\"..\\{$path}\"><Filter>Source\\{$folder}</Filter></{$type}>");
				}
				fecholn($f, "</ItemGroup>");
			}

			fecholn($f, "</Project>");

			fclose($f);
		}
	}

	public function make_vcxproj() {
		foreach($this->projects as $handle=>$info) {
			$f = fopen("{$this->project_dir}/".$info["vcxproj"], "w+");

			fecholn($f, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
			fecholn($f, "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");

			fecholn($f, "<ItemGroup Label=\"ProjectConfigurations\">");
			foreach($this->builds as $build) {
				$fields = explode("|", $build);
				fecholn($f, "<ProjectConfiguration Include=".quote($build).">");
					fecholn($f, "<Configuration>{$fields[0]}</Configuration>");
					fecholn($f, "<Platform>{$fields[1]}</Platform>");
				fecholn($f, "</ProjectConfiguration>");
			}
			fecholn($f, "</ItemGroup>");

			fecholn($f, "<PropertyGroup Label=\"Globals\">");
				fecholn($f, "<ProjectGuid>{$info['guid']}</ProjectGuid>");
				fecholn($f, "<Keyword>Win32Proj</Keyword>");
				fecholn($f, "<RootNamespace>{$this->name}</RootNamespace>");
			fecholn($f, "</PropertyGroup>");

			fecholn($f, "<Import Project=\"$(VCTargetsPath)\Microsoft.Cpp.Default.props\" />");
			foreach($this->builds as $build) {
				$fields = explode("|", $build);
				$configurationmap = array("lib"=>"StaticLibrary", "dll"=>"DynamicLibrary", "util"=>"Application");
				$debuglibmap = array("Release"=>"false", "Debug"=>"true");
				fecholn($f, "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='{$build}'\" Label=\"Configuration\">");
					fecholn($f, "<ConfigurationType>{$configurationmap[$handle]}</ConfigurationType>");
					fecholn($f, "<CharacterSet>MultiByte</CharacterSet>");
					fecholn($f, "<UseDebugLibraries>{$debuglibmap[$fields[0]]}</UseDebugLibraries>");
				fecholn($f, "</PropertyGroup>");
			}

			fecholn($f, "<Import Project=\"$(VCTargetsPath)\Microsoft.Cpp.props\" />");

			fecholn($f, "<ImportGroup Label=\"PropertySheets\">");
				fecholn($f, "<Import Project=\"$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />");
			fecholn($f, "</ImportGroup>");

			fecholn($f, "<PropertyGroup Label=\"UserMacros\" />");

			foreach($this->builds as $label=>$build) {
				$fields = explode("|", $label);
				fecholn($f, "<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='{$build}'\">");
					fecholn($f, "<OutDir>$(SolutionDir)..\\bin\\{$fields[0]}\\{$fields[1]}\\</OutDir>");
					fecholn($f, "<IntDir>$(SolutionDir)..\\build\\{$handle}\\{$fields[0]}\\{$fields[1]}\\</IntDir>");
					if ($handle == "util") {
						fecholn($f, "<TargetName>{$this->name}</TargetName>");
						fecholn($f, "<TargetExt>.exe</TargetExt>");
					} else {
						fecholn($f, "<TargetName>lib{$this->name}</TargetName>");
						fecholn($f, "<TargetExt>.{$handle}</TargetExt>");
					}
				fecholn($f, "</PropertyGroup>");
			}

			$settingsmap = array(
				"Optimization"=>array("Release"=>"MaxSpeed", "Debug"=>"Disabled"),
				"IntrinsicFunctions"=>array("Release"=>"true", "Debug"=>"false"),
				"InlineFunctionExpansion"=>array("Release"=>"AnySuitable", "Debug"=>"Disabled"),
				"FavorSizeOrSpeed"=>array("Release"=>"Speed", "Debug"=>"Neither"),
				"BufferSecurityCheck"=>array("Release"=>"false", "Debug"=>"true"),
				"EnableCOMDATFolding"=>array("Release"=>"true", "Debug"=>"false"),
				"OptimizeReferences"=>array("Release"=>"true", "Debug"=>"false"),
				"SubSystem"=>array("lib"=>"Windows", "dll"=>"Windows", "util"=>"Console"),
				"PreprocessorDefinitions"=>array("lib"=>"LIB_PUBLIC=;", "dll"=>"BUILDING_DLL;LIB_PUBLIC=__declspec(dllexport)", "util"=>"LIB_PUBLIC=;UTILITIES"),
			);
			foreach($this->builds as $build) {
				$fields = explode("|", $build);
				fecholn($f, "<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='{$build}'\">");
					fecholn($f, "<ClCompile>");
						/* static options */
						fecholn($f, "<PrecompiledHeader />");
						fecholn($f, "<WarningLevel>Level4</WarningLevel>");
						fecholn($f, "<WholeProgramOptimization>false</WholeProgramOptimization>");
						fecholn($f, "<AdditionalIncludeDirectories>.\\;..\\driver;..\\driver\\x86;..\\include;..\\extensions;..\\src;</AdditionalIncludeDirectories>");
						fecholn($f, "<ObjectFileName>$(IntDir)dummy\\%(RelativeDir)/</ObjectFileName>");

						/* custom options */
						fecholn($f, "<BufferSecurityCheck>{$settingsmap['BufferSecurityCheck'][$fields[0]]}</BufferSecurityCheck>");
						fecholn($f, "<Optimization>{$settingsmap['Optimization'][$fields[0]]}</Optimization>");
						fecholn($f, "<IntrinsicFunctions>{$settingsmap['IntrinsicFunctions'][$fields[0]]}</IntrinsicFunctions>");
						fecholn($f, "<InlineFunctionExpansion>{$settingsmap['InlineFunctionExpansion'][$fields[0]]}</InlineFunctionExpansion>");
						fecholn($f, "<FavorSizeOrSpeed>{$settingsmap['FavorSizeOrSpeed'][$fields[0]]}</FavorSizeOrSpeed>");
						fecholn($f, "<BufferSecurityCheck>{$settingsmap['BufferSecurityCheck'][$fields[0]]}</BufferSecurityCheck>");
						fecholn($f, "<PreprocessorDefinitions>{$settingsmap['PreprocessorDefinitions'][$handle]};%(PreprocessorDefinitions)</PreprocessorDefinitions>");
					fecholn($f, "</ClCompile>");
					fecholn($f, "<Link>");
						fecholn($f, "<GenerateDebugInformation>true</GenerateDebugInformation>");
						fecholn($f, "<SubSystem>{$settingsmap['SubSystem'][$handle]}</SubSystem>");
						fecholn($f, "<EnableCOMDATFolding>{$settingsmap['EnableCOMDATFolding'][$fields[0]]}</EnableCOMDATFolding>");
						fecholn($f, "<OptimizeReferences>{$settingsmap['OptimizeReferences'][$fields[0]]}</OptimizeReferences>");
					fecholn($f, "</Link>");

					switch ($handle) {
						case "lib":
							fecholn($f, "<Lib>");
								fecholn($f, "<LinkTimeCodeGeneration>false</LinkTimeCodeGeneration>");
							fecholn($f, "</Lib>");
							break;

						case "dll":
							break;
						case "util":
							break;
					}
				fecholn($f, "</ItemDefinitionGroup>");
			}
			fecholn($f, "<Import Project=\"$(VCTargetsPath)\Microsoft.Cpp.targets\" />");

			foreach($info["files"] as $handle) {
				fecholn($f, "<ItemGroup>");
				foreach($this->files[$handle] as $path) {
					$type = $this->fileinfo[$path]["type"];
					$folder = $this->fileinfo[$path]["basepath"];
					$cleanpath = str_replace("../", "", $path);
					$basename = preg_replace("!(.*)\..*$!", "$1", $this->fileinfo[$path]["basename"]);
					if ($type == "CustomBuild") {
						fecholn($f, "<{$type} Include=\"..\\{$path}\">");
							fecholn($f, "<Message>yasm [{$cleanpath}]</Message>");
							fecholn($f, "<Command Condition=\"'$(Platform)'=='Win32'\">yasm -r nasm -p gas -I./ -I../driver -I../driver/x86 -I../extensions -I../include -o $(IntDir)\\{$folder}\\{$basename}.obj -f win32 ..\\{$path}</Command>");
							fecholn($f, "<Command Condition=\"'$(Platform)'=='x64'\">yasm -r nasm -p gas -I./ -I../driver -I../driver/x86 -I../extensions -I../include -o $(IntDir)\\{$folder}\\{$basename}.obj -f win64 ..\\{$path}</Command>");
							fecholn($f, "<Outputs>$(IntDir)\\{$folder}\\{$basename}.obj</Outputs>");
						fecholn($f, "</{$type}>");
					} else {
						fecholn($f, "<{$type} Include=\"..\\{$path}\"></{$type}>");
					}
				}
				fecholn($f, "</ItemGroup>");
			}

			fecholn($f, "</Project>");

			fclose($f);
		}
	}

	public function make_project() {
		$this->build_files();

		$this->fileinfo = array();
		foreach($this->files as $handle=>$list) {
			foreach($list as $path) {
				$basepath = preg_replace("!^(.*)\\\\.*$!", "$1", $path);
				$basename = preg_replace("!^.*\\\\(.*)$!", "$1", $path);
				$this->fileinfo[$path]["basepath"] = $basepath;
				$this->fileinfo[$path]["basename"] = $basename;

				$ext = preg_replace("!^.*\.(.*)$!", "$1", $path);
				switch ($ext) {
					case "c": $type = "ClCompile"; break;
					case "S": $type = "CustomBuild"; break;
					case "inc": $type = "ClHeader"; break;
					case "h": $type = "ClHeader"; break;
				}
				$this->fileinfo[$path]["type"] = $type;
			}
		}

		$this->make_vcxproj();
		$this->make_vcxproj_filters();
	}
	
	public function make() {
		$this->make_sln();
		$this->make_project();
	}
}


class argument {
	var $set, $value;
}


class flag extends argument {
	function flag($flag) {
		global $argc, $argv;

		$this->set = false;

		$flag = "--{$flag}";
		for ($i = 1; $i < $argc; $i++) {
			if ($argv[$i] !== $flag)
				continue;
			$this->value = true;
			$this->set = true;
			return;
		}
	}
}

$disable_yasm = new flag("disable-yasm");

$sln = new vs2010(trim(file_get_contents("project.def")));
$sln->make();


if ($disable_yasm->set) {
	$yasm = "";
} else {
	$yasm = <<<EOS
/* Visual Studio with Yasm 1.2+ */
#define ARCH_X86
#define HAVE_AVX2
#define HAVE_AVX
#define HAVE_XOP
#define HAVE_SSE4_2
#define HAVE_SSE4_1
#define HAVE_SSSE3
#define HAVE_SSE3
#define HAVE_SSE2
#define HAVE_SSE
#define HAVE_MMX

EOS;
}

$asmopt_h = <<<EOS
#ifndef ASMOPT_H
#define ASMOPT_H

#include <stddef.h>

{$yasm}

#if (defined(_M_IX86))
	#define CPU_32BITS
#elif (defined(_M_X64))
	#define CPU_64BITS
#else
	#error This should never happen
#endif

#define HAVE_INT64
#define HAVE_INT32
#define HAVE_INT16
#define HAVE_INT8

#if (_MSC_VER < 1300)
	typedef signed __int64  int64_t; typedef unsigned __int64  uint64_t;
	typedef signed int      int32_t; typedef unsigned int      uint32_t;
	typedef signed short    int16_t; typedef unsigned short    uint16_t;
	typedef signed char      int8_t; typedef unsigned char      uint8_t;
#elif (_MSC_VER < 1600)
	typedef signed __int64  int64_t; typedef unsigned __int64  uint64_t;
	typedef signed __int32  int32_t; typedef unsigned __int32  uint32_t;
	typedef signed __int16  int16_t; typedef unsigned __int16  uint16_t;
	typedef signed __int8    int8_t; typedef unsigned __int8    uint8_t;
#else
	#include <stdint.h>
#endif

#endif /* ASMOPT_H */


EOS;


$asmopt_internal = <<<EOS
#ifndef ASMOPT_INTERNAL_H
#define ASMOPT_INTERNAL_H

#include "asmopt.h"

#define LOCAL_PREFIX3(a,b) a##_##b
#define LOCAL_PREFIX2(a,b) LOCAL_PREFIX3(a,b)
#define LOCAL_PREFIX(n) LOCAL_PREFIX2(PROJECT_NAME,n)
#define PROJECT_NAME %%name%%

/* yasm */
#if (0)
%define PROJECT_NAME %%name%%
#endif

#endif /* ASMOPT_INTERNAL_H */

EOS;

$sln->write_file("%%projectdir%%/asmopt.h", $asmopt_h);
$sln->write_file("%%projectdir%%/asmopt_internal.h", $asmopt_internal);

?>