#include <algorithm>
#include <cassert>
#include <fstream>
#include <string>

#include "RefFiles.h"

std::wstring mbtowstring(const char* mb, std::size_t size)
{
	std::wstring result;

	while (true) {
		wchar_t wc;
		int bytes = std::mbtowc(&wc, mb, size);
		if (bytes > 0) {
			result.push_back(wc);
			mb += bytes;
			size -= bytes;
		}
		else {
			break;
		}
	}

	return result;
}

RefData* RefFiles::getData(const std::string& name) const
{
    if (auto it = m_refs.find(name); it != m_refs.end()) {
		assert(it->second);

		auto& item = *it->second;

		if (auto&& result = item.data.load()) {
			return result.get();
		}

		auto newData = std::make_shared<RefData>();

		nifly::NifFile nif(item.path);
		if (nif.IsValid()) {

			if (auto ed = nif.FindBlockByName<nifly::NiStringExtraData>(DATA_NAME)) {
				newData->smpPath = ed->stringData.get();
			}

			for (nifly::NiNode* node : nif.GetNodes()) {
				if (!node || node == nif.GetRootNode() || m_exclude.contains(node->name.get())) {
					continue;
				}

				auto pair = newData->bones.insert({ node->name.get(), BoneData() });

				if (pair.second) {
					auto parent = nif.GetParentNode(node);
					pair.first->second.parent = parent ? parent->name.get() : std::string();
					pair.first->second.transform = node->GetTransformToParent();
				}
			}
		}

		std::shared_ptr<RefData> stored;
		if (item.data.compare_exchange_strong(stored, newData)) {
			return newData.get();
		}
		else {
			//Someone else beat us to it. Discard our data and keep theirs.
			return stored.get();
		}
    }

    return nullptr;
}

void RefFiles::readExclusions(const std::filesystem::path& path)
{
	std::ifstream file(path);

	char buf[128];

	while (file.getline(buf, sizeof(buf))) {
		m_exclude.insert(buf);
	}
}

void RefFiles::readReferences(const std::filesystem::path& file, const std::filesystem::path& root)
{
	std::ifstream stream(file);

	char buf[256];
	char* end = buf + sizeof(buf);

	while (stream.getline(buf, sizeof(buf))) {

		if (auto eq = std::find(buf, end, '='); eq < end - 1) {

			auto path = std::filesystem::path(mbtowstring(eq + 1, end - eq - 1));
			if (path.is_relative()) {
				path = root / path;
			}

			//Add this reference if it exists and has a .nif extension
			std::string extension = path.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char c) -> char { return std::tolower(c); });
			if (extension == ".nif" && std::filesystem::directory_entry(path).exists()) {
				m_refs.insert({ std::string(buf, eq), std::make_unique<RefFile>(path) });
			}
		}
	}

	std::cout << "Found " << m_refs.size() << " reference file(s)\n";
}
