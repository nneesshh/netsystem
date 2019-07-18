#pragma once

//------------------------------------------------------------------------------
/**
@class ObjectPool

(C) 2016 n.lee
*/
#include <algorithm>
#include <assert.h>

#define OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(_pool, _obj)			\
	do {															\
		for (unsigned int __idx__ = 0; __idx__ < _pool->_total_size; ++__idx__) {		\
			if (_pool->_all_elements[__idx__]._used) {				\
				_obj = &(_pool->_all_elements[__idx__]._data);

#define OBJECT_POOL_ITERATE_USED_OBJECT_END()						\
			}														\
		}															\
	} while(0)

namespace CC_CONTAINER {

	template <typename T>
	class CObjectPool {
	public:
		struct object_pool_node_t {
			T _data;
			bool _used;
		};

		unsigned int			 _base_id;
		unsigned int			 _total_size;
		object_pool_node_t		*_all_elements;

		unsigned int			*_unused_ids;
		unsigned int			 _unused_size;

		static unsigned int		ConvertId(unsigned int base_id, unsigned int total_size, unsigned int id) {
			return 1 + base_id + total_size - id;
		}

		CObjectPool()
			: _base_id(0)
			, _total_size(0)
			, _unused_size(0) {

		}

		~CObjectPool() {
			delete[] _all_elements;
			delete[] _unused_ids;
		}

		void					SafeInit(unsigned int base_id, unsigned int size) {
			unsigned int i;

			_base_id = base_id;
			_total_size = size;
			_unused_size = size;

			_all_elements = new object_pool_node_t[size];
			_unused_ids = new unsigned int[size];

			for (i = 0; i < size; i++) {
				_all_elements[i]._used = false;
				_unused_ids[i] = i + 1;
			}
		}

		unsigned int			SafeAlloc() {
			unsigned int id;

			// eg: _base_id=600000, id=_total_size, objid=600001
			if (0 == _unused_size) return 0;
			id = _unused_ids[_unused_size - 1];
			--_unused_size;
			_all_elements[id - 1]._used = true;
			return ConvertId(_base_id, _total_size, id);
		}

		unsigned int			SafeAlloc(unsigned int objid) {
			unsigned int i, id;

			id = ConvertId(_base_id, _total_size, objid);
			assert(1 <= id && id <= _total_size);

			for (i = 0; i < _unused_size; i++) {
				if (id == _unused_ids[i]) {
					// overwrite unused with tail
					_unused_ids[i] = _unused_ids[_unused_size - 1];
					--_unused_size;
					_all_elements[id - 1]._used = true;
					return objid;
				}
			}
			return 0;
		}

		void					Release(unsigned int objid) {
			ReleaseInternal(objid);
		}

		void					ReleaseInternal(unsigned int objid) {
			unsigned int i, id;

			id = ConvertId(_base_id, _total_size, objid);
			assert(1 <= id && id <= _total_size);
			assert(_all_elements[id - 1]._used);

#ifdef _DEBUG
			// don't free one id two times
			assert(_unused_size < _total_size);

			for (i = 0; i < _unused_size; ++i) {
				if (id == _unused_ids[i]) {
					assert(false);
					return;
				}
			}
#endif
			_all_elements[id - 1]._used = false;
			++_unused_size;
			_unused_ids[_unused_size - 1] = id;
		}

		unsigned int			GetObjidByIndex(unsigned int index) {
			assert(index < _total_size);
			return ConvertId(_base_id, _total_size, index + 1);
		}

		T *						GetByIndex(unsigned int index) {
			assert(index < _total_size);
			return &_all_elements[index]._data;
		}

		T *						Get(unsigned int objid) {
			unsigned int id = ConvertId(_base_id, _total_size, objid);
			assert(id <= _total_size);
			return &_all_elements[id - 1]._data;
		}

		T *						GetFirst() {
			return GetByIndex(_total_size - 1);
		}

		T *						GetLast() {
			return GetByIndex(0);
		}

		unsigned int			GetObjectNum() {
			return _total_size - _unused_size;
		}

		unsigned int			GetTotalSize() {
			return _total_size;
		}
	};
}

/*EOF*/