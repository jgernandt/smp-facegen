#pragma once
#include <atomic>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "NifFile.hpp"

constexpr const char* DATA_NAME = "HDT Skinned Mesh Physics Object";

std::wstring mbtowstring(const char* mb, std::size_t size);

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

	void readExclusions(const std::filesystem::path& path);
	void readReferences(const std::filesystem::path& file, const std::filesystem::path& root);

private:
	std::set<std::string> m_exclude;
	std::map<std::string, RefFile*> m_refs;
	std::map<std::filesystem::path, std::unique_ptr<RefFile>> m_files;
};
