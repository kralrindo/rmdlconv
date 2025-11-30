// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include <pch.h>
#include <algorithm>
#include <cctype>
#include <core/CommandLine.h>
#include <studio/versions.h>
#include <core/utils.h>

const char* pszVersionHelpString = {
	"Please pick the source RMDL version range:\n"
	" 1:  rmdl v8\n"
	" 2:  rmdl v12\n"
	" 3:  rmdl v12.1\n"
	" 4:  rmdl v12.2 - v12.5\n"
	" 5:  rmdl v13 - v13.1\n"
	" 6:  rmdl v14 - v14.1\n"
	"> "
};

const char* pszRSeqVersionHelpString = {
	"Please input the version of your sequence : \n"
	"7:    s0,1,3,4,5,6\n"
	"7.1:  s7,8\n"
	"10:   s9,10,11,12,13,14\n"
	"11:   s15\n"
	"> "
};

static eRMdlSubVersion ParseApexSubVersion(std::string version)
{
	version.erase(std::remove_if(version.begin(), version.end(), ::isspace), version.end());
	std::transform(version.begin(), version.end(), version.begin(), [](unsigned char ch) { return static_cast<char>(::tolower(ch)); });

	bool strippedPrefix = false;
	if (version.rfind("rmdl", 0) == 0)
	{
		version.erase(0, 4);
		strippedPrefix = true;
	}
	if (!version.empty() && version.front() == 'v')
	{
		version.erase(version.begin());
		strippedPrefix = true;
	}

	if (version == "8")
		return eRMdlSubVersion::VERSION_8;
	if (version == "12" || version == "12.0")
		return eRMdlSubVersion::VERSION_12;
	if (version == "12.1" || version == "121")
		return eRMdlSubVersion::VERSION_12_1;
	if (version == "12.2" || version == "122")
		return eRMdlSubVersion::VERSION_12_2;
	if (version == "13" || version == "13.0")
		return eRMdlSubVersion::VERSION_13;
	if (version == "14" || version == "14.0" || version == "14.1" || version == "141")
		return eRMdlSubVersion::VERSION_14;

	if (!strippedPrefix)
	{
		if (version == "1")
			return eRMdlSubVersion::VERSION_8;
		if (version == "2")
			return eRMdlSubVersion::VERSION_12;
		if (version == "3")
			return eRMdlSubVersion::VERSION_12_1;
		if (version == "4")
			return eRMdlSubVersion::VERSION_12_2;
		if (version == "5")
			return eRMdlSubVersion::VERSION_13;
		if (version == "6")
			return eRMdlSubVersion::VERSION_14;
	}

	return eRMdlSubVersion::VERSION_UNK;
}

// move on from this
void LegacyConversionHandling(CommandLine& cmdline)
{
	// using command args
	if (cmdline.argc > 2)
		return;

	if (!FILE_EXISTS(cmdline.argv[1]))
		Error("couldn't find input file\n");

	std::string mdlPath(cmdline.argv[1]);

	BinaryIO mdlIn;
	mdlIn.open(mdlPath, BinaryIOMode::Read);

	if (mdlIn.read<int>() == 'TSDI')
	{
		int mdlVersion = mdlIn.read<int>();

		switch (mdlVersion)
		{
		case MdlVersion::GARRYSMOD:
		{
			uintmax_t mdlFileSize = GetFileSize(mdlPath);

			mdlIn.seek(0, std::ios::beg);

			char* mdlBuf = new char[mdlFileSize];

			mdlIn.getReader()->read(mdlBuf, mdlFileSize);

			ConvertMDL48To54(mdlBuf, mdlPath, mdlPath);

			delete[] mdlBuf;

			break;
		}
		case MdlVersion::PORTAL2:
		{
			uintmax_t mdlFileSize = GetFileSize(mdlPath);

			mdlIn.seek(0, std::ios::beg);

			char* mdlBuf = new char[mdlFileSize];

			mdlIn.getReader()->read(mdlBuf, mdlFileSize);

			ConvertMDL49To54(mdlBuf, mdlPath, mdlPath);

			delete[] mdlBuf;

			break;
		}
		case MdlVersion::TITANFALL:
		{
			uintmax_t mdlFileSize = GetFileSize(mdlPath);

			mdlIn.seek(0, std::ios::beg);

			char* mdlBuf = new char[mdlFileSize];

			mdlIn.getReader()->read(mdlBuf, mdlFileSize);

			ConvertMDL52To53(mdlBuf, mdlPath, mdlPath);

			delete[] mdlBuf;

			break;
		}
		case MdlVersion::TITANFALL2:
		{
			uintmax_t mdlFileSize = GetFileSize(mdlPath);

			mdlIn.seek(0, std::ios::beg);

			char* mdlBuf = new char[mdlFileSize];

			mdlIn.getReader()->read(mdlBuf, mdlFileSize);

			ConvertMDL53To54(mdlBuf, mdlPath, mdlPath);

			delete[] mdlBuf;

			break;
		}
		case MdlVersion::APEXLEGENDS:
		{
			// rmdl subversion
			std::string version = "12.1";

			if (cmdline.HasParam("-version"))
			{
				version = cmdline.GetParamValue("-version", "12.1");
			}
			else
			{
				std::cout << pszVersionHelpString;
				std::cin >> version;
			}

			auto loadModelFile = [&](std::unique_ptr<char[]>& buffer) -> uintmax_t
			{
				const uintmax_t mdlFileSize = GetFileSize(mdlPath);
				mdlIn.seek(0, std::ios::beg);
				buffer.reset(new char[mdlFileSize]);
				mdlIn.getReader()->read(buffer.get(), mdlFileSize);
				return mdlFileSize;
			};

			const eRMdlSubVersion subVersion = ParseApexSubVersion(version);
			if (subVersion == eRMdlSubVersion::VERSION_UNK)
			{
				Error("version '%s' is not currently supported\n", version.c_str());
			}

			const char* const subVersionLabel = DescribeSubVersion(subVersion);
			printf("Input file is RMDL v%s. attempting conversion...\n", subVersionLabel);

			switch (subVersion)
			{
			case eRMdlSubVersion::VERSION_8:
			{
				std::unique_ptr<char[]> mdlBuf;
				loadModelFile(mdlBuf);
				ConvertRMDL8To10(mdlBuf.get(), mdlPath, mdlPath);
				break;
			}
			case eRMdlSubVersion::VERSION_12:
			{
				std::unique_ptr<char[]> mdlBuf;
				const uintmax_t mdlFileSize = loadModelFile(mdlBuf);
				ConvertRMDL120To10(mdlBuf.get(), static_cast<size_t>(mdlFileSize), mdlPath, mdlPath);
				break;
			}
			case eRMdlSubVersion::VERSION_12_1:
			case eRMdlSubVersion::VERSION_12_2:
			case eRMdlSubVersion::VERSION_13:
			case eRMdlSubVersion::VERSION_14:
			{
				std::unique_ptr<char[]> mdlBuf;
				loadModelFile(mdlBuf);
				ConvertRMDL121To10(mdlBuf.get(), mdlPath, mdlPath, subVersion);
				break;
			}
			default:
				Error("version '%s' is not currently supported\n", version.c_str());
				break;
			}

			break;
		}
		default:
		{
			Error("MDL version %i is currently unsupported\n", mdlVersion);
			break;
		}
		}
	}
	else if (mdlPath.find(".rseq"))
	{
		printf("seq gaming\n");

		std::string version = "7.1";

		if (cmdline.HasParam("-version"))
		{
			version = cmdline.GetParamValue("-version", "7.1");
		}
		else
		{
			std::cout << pszRSeqVersionHelpString;
			std::cin >> version;
		}

		uintmax_t seqFileSize = GetFileSize(mdlPath);

		mdlIn.seek(0, std::ios::beg);

		char* seqBuf = new char[seqFileSize];

		mdlIn.getReader()->read(seqBuf, seqFileSize);


		std::string rseqExtPath = ChangeExtension(mdlPath, "rseq_ext");
		char* seqExternalBuf = nullptr;
		if (FILE_EXISTS(rseqExtPath))
		{
			int seqExtFileSize = GetFileSize(rseqExtPath);

			seqExternalBuf = new char[seqExtFileSize];

			std::ifstream ifs(rseqExtPath, std::ios::in | std::ios::binary);

			ifs.read(seqExternalBuf, seqExtFileSize);
		}

		if (version == "7.1")
		{
			//printf("converting rseq version 7.1 to version 7\n");

			ConvertRSEQFrom71To7(seqBuf, seqExternalBuf, mdlPath);
		}
		else if (version == "10")
		{
			ConvertRSEQFrom10To7(seqBuf, seqExternalBuf, mdlPath);
		}

		delete[] seqBuf;
	}
	else
	{
		Error("invalid input file. must be a valid .(r)mdl file with magic 'IDST'\n");
	}
}

int main(int argc, char** argv)
{

	printf("rmdlconv - Copyright (c) %s, Authors: Rexx, Rikayam\n", &__DATE__[7]);

	CommandLine cmdline(argc, argv);

    if (argc < 2)
        Error("invalid usage\n");

	if (cmdline.HasParam("-convertmodel"))
	{
		if (!cmdline.HasParam("-targetversion"))
			Error("no '-targetversion' param found while trying to convert model(s)!!!\n required for proper conversion, exiting...\n");

		std::string modelPath = cmdline.GetParamValue("-convertmodel");
		int modelVersionTarget = atoi(cmdline.GetParamValue("-targetversion"));

		const char* customDir = nullptr; // custom base folder for models

		if (cmdline.HasParam("-outputdir"))
			customDir = cmdline.GetParamValue("-outputdir");

		eRMdlSubVersion cliSubVersion = eRMdlSubVersion::VERSION_12_1;
		if (modelVersionTarget == MdlVersion::APEXLEGENDS && cmdline.HasParam("-version"))
		{
			const char* versionArg = cmdline.GetParamValue("-version");
			cliSubVersion = ParseApexSubVersion(versionArg);
			if (cliSubVersion == eRMdlSubVersion::VERSION_UNK)
				Error("version '%s' is not currently supported\n", versionArg);
		}

		UpgradeStudioModel(modelPath, modelVersionTarget, customDir, cliSubVersion);
	}

	if (cmdline.HasParam("-convertsequence"))
	{
		// todo
	}

	LegacyConversionHandling(cmdline); // this should be cut eventually

	if(!cmdline.HasParam("-nopause"))
		std::system("pause");

	return 0;
}
