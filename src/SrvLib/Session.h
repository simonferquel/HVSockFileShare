#pragma once
#include <Common/SafeSocket.h>
namespace HVFiles {
	class Session {
		
	private:
		GUID _id;
		SafeSocket _s;
	public:
		Session(const GUID& id, const SafeSocket& s);
		void Start();
		GUID id() const { return _id; }
	};
}