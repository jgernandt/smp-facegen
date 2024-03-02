#include <cassert>

#include "Processor.h"

#include "NifFile.hpp"


static bool hasSMPPath(const nifly::NiHeader& hdr, nifly::NiNode* root, const std::string& value)
{
	assert(root);

	for (auto obj : root->extraDataRefs) {
		auto data = hdr.GetBlock(obj);
		if (data && data->name.get() == DATA_NAME) {
			auto stringData = dynamic_cast<nifly::NiStringExtraData*>(data);
			if (stringData && stringData->stringData.get() == value) {
				return true;
			}
		}
	}

	return false;
}

void printThreaded(const std::string& str)
{
	static std::mutex mutex;
	std::lock_guard<decltype(mutex)> lock(mutex);

	std::cout << str << '\n';
}

Processor::Processor(int threads, const RefFiles& refs) :
	m_refs{ refs }
{
	m_threads.resize(threads);
	for (auto&& thread : m_threads) {
		thread = std::thread(&Processor::worker, this);
	}
}

Processor::~Processor()
{
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		m_tasks = std::queue<Task>();
	}

	join();
}

void Processor::join()
{
	//no further tasks will be added after this call is made, so we don't need
	//any special flag for the workers. They will continue until the queue is empty
	//and then return.
	m_signal.release(m_threads.size());

	for (auto&& thread : m_threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

void Processor::process(const std::filesystem::directory_entry& entry,
	const std::filesystem::path& iroot, const std::filesystem::path& oroot)
{
	for (auto&& child : std::filesystem::directory_iterator(entry)) {
		if (child.is_directory()) {
			process(child, iroot, oroot);
		}
		else {
			std::filesystem::path rel = relative(child, iroot);
			push(child, oroot / relative(child, iroot));
		}
	}
}

void Processor::process_nif(const std::filesystem::path& in, const std::filesystem::path& out)
{
	nifly::NifFile nif(in);
	if (nif.IsValid()) {

		bool save = false;

		nifly::NiNode* rootNode = nif.GetRootNode();

		if (rootNode) {
			for (nifly::NiShape* shape : nif.GetShapes()) {
				if (auto ref = m_refs.getData(shape->name.get())) {

					//add extra data unless it exists
					if (!hasSMPPath(nif.GetHeader(), rootNode, ref->smpPath)) {
						auto smpData = std::make_unique<nifly::NiStringExtraData>();
						smpData->name.get() = DATA_NAME;
						smpData->stringData.get() = ref->smpPath;
						nif.AssignExtraData(rootNode, std::move(smpData));
						save = true;
					}

					//correct bone transforms and hierarchy
					std::vector<std::string> bones;
					nif.GetShapeBoneList(shape, bones);

					for (auto&& bone : bones) {

						if (auto it2 = ref->bones.find(bone); it2 != ref->bones.end()) {
							auto& boneData = it2->second;
							auto node = nif.FindBlockByName<nifly::NiNode>(bone);
							if (node) {
								node->SetTransformToParent(boneData.transform);

								auto parent = !boneData.parent.empty() ? nif.FindBlockByName<nifly::NiNode>(boneData.parent) : nullptr;
								if (parent) {
									nif.SetParentNode(node, parent);
								}
								save = true;
							}
						}
					}
				}
			}
		}

		if (save) {
			std::filesystem::create_directories(out.parent_path());
			nif.Save(out);

			printThreaded("Created file " + out.string());
			++m_count;
		}
	}
}

void Processor::push(const std::filesystem::path& in, const std::filesystem::path& out)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	m_tasks.push({ in, out });
	m_signal.release(1);
}

void Processor::worker()
{
	while (true) {
		m_signal.acquire();

		//They release one signal for every added task and one per thread when they make the join call.
		//Every loop here consumes one task.
		//That means that if the queue is empty when we get here, the join call has been made and we are done.

		Task task;
		{
			std::lock_guard<decltype(m_mutex)> lock(m_mutex);

			if (m_tasks.empty()) {
				break;
			}
			else {
				task = m_tasks.front();
				m_tasks.pop();
			}
		}

		try {
			process_nif(task.in, task.out);
		}
		catch (const std::exception& e) {
			printThreaded(e.what());
		}
	}
}
