#pragma once
#include <atomic>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "NifFile.hpp"

constexpr const char* DATA_NAME = "HDT Skinned Mesh Physics Object";

struct BoneData
{
	std::string parent;
	nifly::MatTransform transform;
};

struct RefData
{
	std::string smpPath;
	std::map<std::string, BoneData> bones;
};

class RefFiles
{
private:
	struct RefFile
	{
		std::filesystem::path path;
		std::atomic<std::shared_ptr<RefData>> data;
	};

public:
	RefFiles() = default;
	~RefFiles() = default;
	RefFiles(const RefFiles&) = delete;

	RefData* getData(const std::string& name) const;

	void readDirectory(const std::filesystem::path& dir);
	void readExclusions(const std::filesystem::path& path);

private:
	std::set<std::string> m_exclude;
	std::map<std::string, std::unique_ptr<RefFile>> m_refs;
};
