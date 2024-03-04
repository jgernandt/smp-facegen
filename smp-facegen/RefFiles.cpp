#include <cassert>
#include <fstream>
#include <string>

#include "RefFiles.h"

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

void RefFiles::readDirectory(const std::filesystem::path& dir)
{
	for (auto&& entry : std::filesystem::directory_iterator(dir)) {
		std::string extension = entry.path().extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c) -> char { return std::tolower(c); });
		if (extension == ".nif") {
			m_refs.insert({ entry.path().stem().string(), std::make_unique<RefFile>(entry) });
		}
	}
}

void RefFiles::readExclusions(const std::filesystem::path& path)
{
	std::ifstream file(path);

	char buf[128];

	while (file.getline(buf, sizeof(buf))) {
		m_exclude.insert(buf);
	}
}
