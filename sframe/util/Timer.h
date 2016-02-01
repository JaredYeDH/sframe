
#ifndef SFRAME_TIMER_H
#define SFRAME_TIMER_H

#include <assert.h>
#include <inttypes.h>
#include <set>
#include <vector>
#include <functional>

namespace sframe {

//// ��ʱ������
//enum TimerType : int32_t
//{
//	kTimerType_NormalTimer = 1,
//	kTimerType_ObjectTimer,
//};
//
//// ��ʱ��
//class Timer
//{
//	friend class TimerCompare;
//public:
//	Timer(int32_t timer_id, int64_t exe_time) : _timer_id(timer_id), _execute_time(exe_time) {}
//
//	virtual ~Timer() {}
//
//	int32_t GetTimerId() const
//	{
//		return _timer_id;
//	}
//
//	int64_t GetExecuteTime() const
//	{
//		return _execute_time;
//	}
//
//	void SetExeceteTime(int64_t exe_time)
//	{
//		_execute_time = exe_time;
//	}
//
//	virtual TimerType GetType() const = 0;
//
//	virtual int32_t Invoke(int64_t cur_time) const = 0;
//
//private:
//	int32_t _timer_id;
//	int64_t _execute_time;  // ִ��ʱ��
//};
//
//// ��ʱ���Ƚ���
//class TimerCompare
//{
//public:
//	bool operator()(const Timer * timer1, const Timer * timer2)
//	{
//		return timer1->_execute_time < timer2->_execute_time;
//	}
//};
//
//// ��ͨ��ʱ��
//class NormalTimer : public Timer
//{
//public:
//
//	typedef std::function<int32_t(int64_t)> Func;
//
//	NormalTimer(int32_t timer_id, int64_t exe_time, const Func & func ) : Timer(timer_id, exe_time), _func(func)
//	{
//		assert(_func);
//	}
//
//	~NormalTimer() {}
//
//	TimerType GetType() const override
//	{
//		return kTimerType_NormalTimer;
//	}
//
//	int32_t Invoke(int64_t cur_time) const override
//	{
//		return _func(cur_time);
//	}
//
//private:
//	Func _func;
//};
//
//// ����ʱ��
//template<typename T_ObjKey, typename T_Obj>
//class ObjectTimer : public Timer
//{
//public:
//	// ���������
//	typedef std::function<T_Obj*(T_ObjKey)> ObjectFinder;
//	// ִ�еĺ���
//	typedef int32_t(T_Obj::*Func)(int64_t);
//
//	ObjectTimer(int32_t timer_id, int64_t exe_time, const ObjectFinder & obj_finder, Func func) 
//		: Timer(timer_id, exe_time), _obj_finder(obj_finder), _func(func)
//	{
//		assert(_obj_finder && _func);
//	}
//
//	~ObjectTimer(){}
//
//	TimerType GetType() const override
//	{
//		return kTimerType_ObjectTimer;
//	}
//
//	int32_t Invoke(int64_t cur_time) const override
//	{
//		T_Obj *obj = _obj_finder(t->obj_key);
//		if (obj == nullptr)
//		{
//			return -1;
//		}
//
//		int32_t after_ms = (obj->*(t->func))(cur_time);
//		return after_ms;
//	}
//
//private:
//	ObjectFinder _obj_finder;
//	Func _func;
//};
//
//// ��ʱ��������
//class TimerManager
//{
//public:
//	TimerManager() {}
//
//	~TimerManager() 
//	{
//		for (auto it = _timer.begin(); it != _timer.end(); it++)
//		{
//			delete (*it);
//		}
//	}
//
//private:
//	std::multiset<Timer*> _timer;
//};

// ����ʱ��������
template<typename T_ObjKey, typename T_Obj>
class ObjectTimerManager
{
public:

	// Timer����
	// ���뵱ǰʱ��
	// �����´ζ�ú�ִ�У�С��0Ϊֹͣ��ʱ
	typedef int32_t(T_Obj::*TimerFunc)(int64_t);

	// ���������
	typedef std::function<T_Obj*(T_ObjKey)> ObjectFindor; 

	struct Timer
	{
		int32_t timer_id;
		int64_t execut_time;  // ִ��ʱ��
		T_ObjKey obj_key;
		TimerFunc func;

		bool operator<(const Timer & t) const 
		{
			return this->execut_time < t.execut_time;
		}
	};

public:
	// executor: ִ�з����������´ζ��ٺ����ִ�У�С��0Ϊֹͣ��ǰ��timer
	ObjectTimerManager(ObjectFindor obj_finder)
		: _cur_max_timer_id(0), _obj_finder(obj_finder), _executing(false)
	{
		_temp.reserve(1024);
	}
	
	~ObjectTimerManager() {}

	// ע��
	// exe_time: ִ��ʱ�䣨����ʱ�䣩
	// ���ش���0��id��������ʧ��
	int32_t Regist(int64_t exe_time, T_ObjKey obj_key, TimerFunc func)
	{
		Timer t;
		t.timer_id = ++_cur_max_timer_id;
		t.obj_key = obj_key;
		t.execut_time = exe_time;
		t.func = func;

		if (_executing)
		{
			_temp.push_back(t);
		}
		else
		{
			_timer.insert(t);
		}

		if (_cur_max_timer_id == 0x7fffffff)
		{
			_cur_max_timer_id = 0;
		}

		return t.timer_id;
	}

	// ɾ����ʱ��
	void DeleteTimer(int32_t timer_id)
	{
		for (auto it = _timer.begin(); it != _timer.end(); it++)
		{
			if (it->timer_id == timer_id)
			{
				_timer.erase(it);
				return;
			}
		}

		for (auto it = _temp.begin(); it < _temp.end(); it++)
		{
			if (it->timer_id == timer_id)
			{
				_temp.erase(it);
				return;
			}
		}
	}

	// ִ��
	void Execute(int64_t cur_time)
	{
		_executing = true;

		while (!_timer.empty())
		{
			auto it = _timer.begin();
			if (it->execut_time >= cur_time)
			{
				break;
			}

			Timer t = (*it);
			_timer.erase(it);

			T_Obj *obj = _obj_finder(t.obj_key);
			if (obj == nullptr)
			{
				continue;
			}

			int32_t after_ms = (obj->*(t.func))(cur_time);
			if (after_ms >= 0)
			{
				t.execut_time = cur_time + after_ms;
				_temp.push_back(t);
			}
		}

		_executing = false;

		// ��temp�е�timer���¼����ִ��timer����
		for (auto it = _temp.begin(); it < _temp.end(); it++)
		{
			_timer.insert(*it);
		}

		_temp.clear();
	}

private:
	int32_t _cur_max_timer_id;
	ObjectFindor _obj_finder;
	std::multiset<Timer> _timer;
	std::vector<Timer> _temp;
	bool _executing;
};

}

#endif