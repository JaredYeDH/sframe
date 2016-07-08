
#ifndef SFRAME_TIMER_H
#define SFRAME_TIMER_H

#include <assert.h>
#include <inttypes.h>
#include <vector>
#include <memory>

namespace sframe {

class Timer;

class TimerWrapper
{
	friend class Timer;
	friend class TimerManager;
	friend struct TimerGroup;
public:
	TimerWrapper(Timer * timer) : _timer(timer), _group(-1) {}

	bool IsAlive() const { return _timer != nullptr; }

private:
	void SetDeleted() { _timer = nullptr; }

	void SetGroup(int32_t group) { _group = group; }

	Timer * GetTimerPtr() { return _timer; }

	int32_t GetGroup() const { return _group; }

	Timer * _timer;
	int32_t _group;
};

typedef std::shared_ptr<TimerWrapper> TimerHandle;

// ��ʱ��
class Timer
{
public:

	Timer() : _exec_time(0), _prev(nullptr), _next(nullptr)
	{
		_handle = std::make_shared<TimerWrapper>(this);
	}

	virtual ~Timer()
	{
		_handle->SetDeleted();
	}

	const TimerHandle & GetHandle() const
	{
		return _handle;
	}

	void SetExecTime(int64_t exec_time)
	{
		_exec_time = exec_time;
	}

	int64_t GetExecTime() const
	{
		return _exec_time;
	}

	Timer * GetPrev() const
	{
		return _prev;
	}

	Timer * GetNext() const
	{
		return _next;
	}

	void SetPrev(Timer * t)
	{
		_prev = t;
	}

	void SetNext(Timer * t)
	{
		_next = t;
	}

	virtual int64_t Invoke() const = 0;

protected:
	TimerHandle _handle;
	int64_t _exec_time;     // ִ��ʱ��
	Timer * _prev;
	Timer * _next;
};

// ��ͨTimer(ִ�о�̬����)
class NormalTimer : public Timer
{
public:
	// �����´ζ�ú�ִ�У�С��0Ϊֹͣ��ʱ��
	typedef int64_t(*TimerFunc)();

	NormalTimer(TimerFunc func) : _func(func) {}

	virtual ~NormalTimer() {}

	// ִ��
	int64_t Invoke() const override
	{
		int64_t next = -1;
		if (_func)
		{
			next = (*_func)();
		}
		return next;
	}

private:
	TimerFunc _func;
};

// ��ȫ��ʱ�����󣬰�װһ����������ʵ�ְ�ȫ��Timer(�����ͷŶ���󣬲����ֶ�ɾ��Timer��Ҳ�ǰ�ȫ��)
template<typename T_Obj>
class SafeTimerObj
{
public:
	SafeTimerObj() : _obj_ptr(nullptr) {}

	void SetObjectPtr(T_Obj * obj_ptr)
	{
		_obj_ptr = obj_ptr;
	}

	T_Obj * GetObjectPtr() const
	{
		return _obj_ptr;
	}

private:
	T_Obj * _obj_ptr;
};

// shared_ptr�ػ�
template<typename T_Obj>
class SafeTimerObj<std::shared_ptr<T_Obj>>
{
public:
	SafeTimerObj() {}

	void SetObjectPtr(const std::shared_ptr<T_Obj> & obj_ptr)
	{
		_obj_ptr = obj_ptr;
	}

	T_Obj * GetObjectPtr() const
	{
		return _obj_ptr.get();
	}

private:
	std::shared_ptr<T_Obj> _obj_ptr;
};

// ��ʱ���������
template<typename T_Obj>
struct TimerObjHelper
{
};

// ԭʼָ���ػ�
template<typename T_Obj>
struct TimerObjHelper<T_Obj*>
{
	typedef T_Obj ObjectType;

	static T_Obj * GetOriginalPtr(T_Obj * obj_ptr)
	{
		return obj_ptr;
	}
};

// shared_ptr�ػ�
template<typename T_Obj>
struct TimerObjHelper<std::shared_ptr<T_Obj>>
{
	typedef T_Obj ObjectType;

	static T_Obj * GetOriginalPtr(const std::shared_ptr<T_Obj> & obj_ptr)
	{
		return obj_ptr.get();
	}
};

// shared_ptr<SafeTimerObj>�ػ�
template<typename T_Obj>
struct TimerObjHelper<std::shared_ptr<SafeTimerObj<T_Obj>>>
{
	typedef T_Obj ObjectType;

	static T_Obj * GetOriginalPtr(const std::shared_ptr<SafeTimerObj<T_Obj>> & obj_ptr)
	{
		return obj_ptr->GetObjectPtr();
	}
};

// SafeTimerObj�ػ���������ֱ��ʹ��SafeTimerObj*��
template<typename T_Obj>
struct TimerObjHelper<SafeTimerObj<T_Obj>*>
{
};

// ����Timer(ִ�ж��󷽷�)
template<typename T_ObjPtr>
class ObjectTimer : public Timer
{
public:
	typedef typename TimerObjHelper<T_ObjPtr>::ObjectType TimerObjType;

	// �����´ζ�ú�ִ�У�С��0Ϊֹͣ��ʱ��
	typedef int64_t(TimerObjType::*TimerFunc)();

	ObjectTimer(const T_ObjPtr &  obj_ptr, TimerFunc func) : _obj_ptr(obj_ptr), _func(func) {}

	virtual ~ObjectTimer() {}

	// ִ��
	int64_t Invoke() const override
	{
		int64_t next = -1;
		if (_func)
		{
			TimerObjType * origin_ptr = TimerObjHelper<T_ObjPtr>::GetOriginalPtr(_obj_ptr);
			if (origin_ptr)
			{
				next = (origin_ptr->*(_func))();
			}
		}
		return next;
	}

private:
	T_ObjPtr _obj_ptr;
	TimerFunc _func;
};

// ��ʱ����
struct TimerGroup
{
	TimerGroup() : timer_head(nullptr), timer_tail(nullptr), min_exec_time(0) {}

	void DeleteTimer(Timer * t)
	{
		if (t == nullptr)
		{
			return;
		}

		Timer * prev = t->GetPrev();
		Timer * next = t->GetNext();
		if (prev && next)
		{
			assert(t != timer_head && t != timer_tail);
			prev->SetNext(next);
			next->SetPrev(prev);
		}
		else if (prev)
		{
			assert(t == timer_tail);
			timer_tail = prev;
			timer_tail->SetNext(nullptr);
		}
		else if (next)
		{
			assert(t == timer_head);
			timer_head = next;
			timer_head->SetPrev(nullptr);
		}
		else
		{
			assert(t == timer_head && t == timer_tail);
			timer_head = nullptr;
			timer_tail = nullptr;
			min_exec_time = 0;
		}

		t->SetPrev(nullptr);
		t->SetNext(nullptr);
		t->GetHandle()->SetGroup(-1);
	}

	void AddTimer(Timer * t)
	{
		assert(t->GetPrev() == nullptr && t->GetNext() == nullptr);
		if (timer_tail == nullptr)
		{
			assert(timer_head == nullptr);
			timer_head = t;
			timer_tail = t;
		}
		else
		{
			t->SetPrev(timer_tail);
			timer_tail->SetNext(t);
			timer_tail = t;
		}
	}

	bool IsEmpty() const
	{
		return timer_head == nullptr;
	}

	Timer* timer_head;           // ��ʱ������ͷ
	Timer* timer_tail;           // ��ʱ������ͷ
	int64_t min_exec_time;       // ��С��ִ��ʱ��
};

// ��ʱ���������������̣߳�
class TimerManager
{
public:
	static const int32_t kAllTimerGroupsNumber = 120;        // ����ʱ����Ϊ������
	static const int32_t kTimeSpanOneGroup = 1;              // һ����ʱ�����ʱ���ȣ��� S��

																// executor: ִ�з����������´ζ��ٺ����ִ�У�С��0Ϊֹͣ��ǰ��timer
	TimerManager() : _cur_group(0), _cur_exec_timer(nullptr)
	{
		_add_timer_cache.reserve(128);
	}

	~TimerManager();

	// ע����ͨ��ʱ��
	// after_msec: ���ٺ����ִ��
	TimerHandle RegistNormalTimer(int64_t after_msec, NormalTimer::TimerFunc func);

	// ע�����ʱ��
	template<typename T_ObjPtr>
	TimerHandle RegistObjectTimer(int64_t after_msec, typename ObjectTimer<T_ObjPtr>::TimerFunc func, const T_ObjPtr & obj_ptr)
	{
		if (!func || after_msec < 0)
		{
			assert(false);
			return 0;
		}

		int64_t now = Now();
		if (_cur_group_change_time <= 0)
		{
			_cur_group_change_time = now;
		}

		ObjectTimer<T_ObjPtr> * t = new ObjectTimer<T_ObjPtr>(obj_ptr, func);
		t->SetExecTime(now + after_msec);
		AddTimer(t);

		return t->GetHandle();
	}

	// ɾ����ʱ��
	void DeleteTimer(TimerHandle timer_handle);

	// ִ��
	void Execute();

private:
	void AddTimer(Timer * t);

	int64_t Now();

private:
	TimerGroup _timers_groups[kAllTimerGroupsNumber];    // ���еĶ�ʱ����
	std::vector<Timer*> _add_timer_cache;                // ��Ӷ�ʱ������
	int32_t _cur_group;
	int64_t _cur_group_change_time;
	Timer * _cur_exec_timer;
};


// ��ȫTimerע�ᣬ�������࣬����ע�ᶨʱ�����������������ֶ�ɾ����ʱ��
template<typename T>
class SafeTimerRegistor
{
public:
	SafeTimerRegistor() : _timer_mgr(nullptr)
	{
		static_assert(std::is_base_of<SafeTimerRegistor, T>::value, "T must derived from SafeTimerRegistor");
	}

	virtual ~SafeTimerRegistor()
	{
		if (_safe_timer_obj)
		{
			_safe_timer_obj->SetObjectPtr(nullptr);
		}
	}

	void SetTimerManager(TimerManager * timer_mgr)
	{
		_timer_mgr = timer_mgr;
	}

	TimerManager * GetTimerManager() const
	{
		return _timer_mgr;
	}

	// ע�ᶨʱ��(ֻ��ע����������)
	TimerHandle RegistTimer(int64_t after_msec, typename ObjectTimer<T*>::TimerFunc func)
	{
		if (_timer_mgr == nullptr)
		{
			assert(false);
			return 0;
		}

		if (!_safe_timer_obj)
		{
			_safe_timer_obj = std::make_shared<SafeTimerObj<T>>();
			_safe_timer_obj->SetObjectPtr(static_cast<T*>(this));
		}

		return _timer_mgr->RegistObjectTimer(after_msec, func, _safe_timer_obj);
	}

private:
	std::shared_ptr<SafeTimerObj<T>> _safe_timer_obj;
	TimerManager * _timer_mgr;
};

}

#endif