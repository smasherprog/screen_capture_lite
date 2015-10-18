#include "stdafx.h"
#include "Screen.h"



namespace SL {
	namespace Screen_Capture {
		struct Blk {
			size_t size = 0;
			char* data = nullptr;
		};
	}
}

class BufferManager {
	size_t _Bytes_Allocated = 0;
	const size_t MAX_ALLOCATED_BYTES = 1024 * 1024 * 64;//64 MB MAX
	std::vector<std::shared_ptr<SL::Screen_Capture::Blk>> _Buffer;
	std::mutex _BufferLock;
public:
	std::shared_ptr<SL::Screen_Capture::Blk> AquireBuffer(size_t req_bytes) {
		std::lock_guard<std::mutex> lock(_BufferLock);
		auto found = std::remove_if(begin(_Buffer), end(_Buffer), [=](std::shared_ptr<SL::Screen_Capture::Blk>& c) {  return c->size >= req_bytes; });

		if (found == _Buffer.end()) {
			auto b = new SL::Screen_Capture::Blk;
			b->data = new char[req_bytes];
			b->size = req_bytes;
			return std::shared_ptr<SL::Screen_Capture::Blk>(b, [](SL::Screen_Capture::Blk* b) { delete[] b->data; delete b; });
		}
		else {
			auto ret(*found);
			_Bytes_Allocated -= ret->size;
			_Buffer.erase(found);
			return ret;
		}
	}
	void ReleaseBuffer(std::shared_ptr<SL::Screen_Capture::Blk>& buffer) {
		if (buffer == nullptr) return;
		if (_Bytes_Allocated < MAX_ALLOCATED_BYTES) {//ignore the fact that this will mean our buffer holds more than our maxsize
			std::lock_guard<std::mutex> lock(_BufferLock);
			auto found = std::find(begin(_Buffer), end(_Buffer), buffer);
			if (found == _Buffer.end()) {
				_Bytes_Allocated += buffer->size;
				_Buffer.emplace_back(buffer);
			}
		}//otherwise ignore and let it be reclaimed
	}
};

static BufferManager Buffer_Manager;

SL::Screen_Capture::Image::~Image() {
	Buffer_Manager.ReleaseBuffer(_Data);
}
char * SL::Screen_Capture::Image::getData() const
{
	return _Data->data;
}
std::shared_ptr<SL::Screen_Capture::Blk> SL::Screen_Capture::AquireBuffer(size_t req_bytes) {
	return Buffer_Manager.AquireBuffer(req_bytes);
}
void SL::Screen_Capture::ReleaseBuffer(std::shared_ptr<Blk>& buffer) {
	Buffer_Manager.ReleaseBuffer(buffer);
}
size_t SL::Screen_Capture::getSize(const std::shared_ptr<Blk>& b)
{
	return b->size;
}

char * SL::Screen_Capture::getData(const std::shared_ptr<Blk>& b)
{
	return b->data;
}