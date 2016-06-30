
#ifndef SFRAME_TIMER_H
#define SFRAME_TIMER_H

#include <assert.h>
#include <inttypes.h>
#include <vector>
#include <list>
#include <functional>

namespace sframe {

// ����ʱ���������������̣߳�
template<typename T_ObjKey, typename T_Obj>
class ObjectTimerManager
{
public:

	// Timer����
	// ���뵱ǰʱ��
	// �����´ζ�ú�ִ�У�С�ڵ���0Ϊֹͣ��ʱ��
	typedef int32_t(T_Obj::*TimerFunc)(int64_t);

	// ���������
	typedef std::function<T_Obj*(T_ObjKey)> ObjectFindor; 

	struct Timer
	{
		uint32_t timer_id;
		int64_t execut_time;  // ִ��ʱ��
		T_ObjKey obj_key;
		TimerFunc func;
		bool del;

		bool operator == (const Timer & t) const 
		{
			return this->timer_id == t.timer_id;
		}
	};

public:
	// executor: ִ�з����������´ζ��ٺ����ִ�У�С��0Ϊֹͣ��ǰ��timer
	ObjectTimerManager(ObjectFindor obj_finder)
		: _cur_max_timer_id(0), _obj_finder(obj_finder), _executing(false), _min_exec_time(0)
	{
		_add_temp.reserve(512);
	}
	
	~ObjectTimerManager() {}

	// ע��
	// exe_time: ִ��ʱ�䣨����ʱ�䣩
	// ���ش���0��id��������ʧ��
	uint32_t Regist(int64_t exe_time, T_ObjKey obj_key, TimerFunc func)
	{
		Timer t;
		t.timer_id = NewTimerId();
		t.obj_key = obj_key;
		t.execut_time = exe_time;
		t.func = func;
		t.del = false;

		if (_executing)
		{
			_add_temp.push_back(t);
		}
		else
		{
			_timer.push_back(t);
			if (_min_exec_time <= 0 || exe_time < _min_exec_time)
			{
				_min_exec_time = exe_time;
			}
		}

		return t.timer_id;
	}

	// ɾ����ʱ��
	void DeleteTimer(uint32_t timer_id)
	{
		Timer find_timer;
		find_timer.timer_id = timer_id;

		if (_executing)
		{
			auto it_timer = std::find(_timer.begin(), _timer.end(), find_timer);
			if (it_timer != _timer.end())
			{
				it_timer->del = true;
			}
			else
			{
				auto it_tmp = std::find(_add_temp.begin(), _add_temp.end(), find_timer);
				if (it_tmp != _add_temp.end())
				{
					_add_temp.erase(it_tmp);
				}
			}
		}
		else
		{
			auto it = std::find(_timer.begin(), _timer.end(), find_timer);
			if (it != _timer.end())
			{
				_timer.erase(it);
			}
		}
	}

	// ִ��
	void Execute(int64_t cur_time)
	{
		if (_min_exec_time <= 0 || cur_time < _min_exec_time)
		{
			return;
		}

		_min_exec_time = 0;
		_executing = true;
		assert(_add_temp.empty());
		auto it = _timer.begin();

		while (it != _timer.end())
		{
			Timer & cur = *it;
			if (cur.del)
			{
				it = _timer.erase(it);
				continue;
			}

			if (cur_time >= cur.execut_time)
			{
				T_Obj *obj = _obj_finder(cur.obj_key);
				if (obj)
				{
					int32_t after_ms = (obj->*(cur.func))(cur_time);
					if (after_ms <= 0)
					{
						it = _timer.erase(it);
						continue;
					}
					else
					{
						cur.execut_time = cur_time + after_ms;
						if (_min_exec_time <= 0 || cur.execut_time < _min_exec_time)
						{
							_min_exec_time = cur.execut_time;
						}
					}
				}
				else
				{
					it = _timer.erase(it);
					continue;
				}
			}
			
			it++;
		}

		_executing = false;

		// ��add_temp�е�timer����timer�б�
		for (Timer & t : _add_temp)
		{
			_timer.push_back(t);
			if (_min_exec_time <= 0 || t.execut_time < _min_exec_time)
			{
				_min_exec_time = t.execut_time;
			}
		}
		_add_temp.clear();
	}

private:
	uint32_t NewTimerId()
	{
		if (_cur_max_timer_id == 0xffffffff)
		{
			_cur_max_timer_id = 0;
		}

		return ++_cur_max_timer_id;
	}

private:
	uint32_t _cur_max_timer_id;
	ObjectFindor _obj_finder;
	std::list<Timer> _timer;
	std::vector<Timer> _add_temp;
	int64_t _min_exec_time;
	bool _executing;
};

}

#endif