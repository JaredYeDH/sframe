
#ifndef SFRAME_TIMER_H
#define SFRAME_TIMER_H

#include <assert.h>
#include <inttypes.h>
#include <vector>
#include <list>
#include <memory>

namespace sframe {

// ��ʱ��
class Timer
{
public:
	typedef uint32_t ID;

	Timer(ID id) : _timer_id(id), _exec_time(0), _del(false) {}

	virtual ~Timer() {}

	ID GetTimerID() const
	{
		return _timer_id;
	}

	void SetExecTime(int64_t exec_time)
	{
		_exec_time = exec_time;
	}

	int64_t GetExecTime() const
	{
		return _exec_time;
	}

	void SetDelete()
	{
		_del = true;
	}

	bool IsDeleted() const
	{
		return _del;
	}

	bool operator == (const Timer & t) const
	{
		return this->_timer_id == t._timer_id;
	}

	virtual int64_t Invoke() const
	{
		return 0;
	}

protected:
	ID _timer_id;
	int64_t _exec_time;  // ִ��ʱ��
	bool _del;
};

// ��ͨTimer(ִ�о�̬����)
class NormalTimer : public Timer
{
public:
	// �����´ζ�ú�ִ�У�С��0Ϊֹͣ��ʱ��
	typedef int64_t(*TimerFunc)();

	NormalTimer(ID id, TimerFunc func) : Timer(id), _func(func) {}

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

	ObjectTimer(ID id, const T_ObjPtr &  obj_ptr, TimerFunc func) : Timer(id), _obj_ptr(obj_ptr), _func(func) {}

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

// ��ʱ���������������̣߳�
class TimerManager
{
public:
	// executor: ִ�з����������´ζ��ٺ����ִ�У�С��0Ϊֹͣ��ǰ��timer
	TimerManager(bool use_steady_time = true) : 
		_use_steady_time(use_steady_time), _cur_max_timer_id(0), _executing(false), _min_exec_time(0), _check_existed_when_new_id(false)
	{
		_add_temp.reserve(512);
	}

	~TimerManager();

	// ע����ͨ��ʱ��
	// after_msec: ���ٺ����ִ��
	// ���ش���0��id
	Timer::ID RegistNormalTimer(int64_t after_msec, NormalTimer::TimerFunc func);

	// ע�����ʱ��
	template<typename T_ObjPtr>
	Timer::ID RegistObjectTimer(int64_t after_msec, typename ObjectTimer<T_ObjPtr>::TimerFunc func, const T_ObjPtr & obj_ptr)
	{
		if (!func || after_msec < 0)
		{
			assert(false);
			return 0;
		}

		ObjectTimer<T_ObjPtr> * t = new ObjectTimer<T_ObjPtr>(NewTimerId(), obj_ptr, func);
		t->SetExecTime(Now() + after_msec);
		AddTimer(t);

		return t->GetTimerID();
	}

	// ɾ����ʱ��
	void DeleteTimer(Timer::ID timer_id);

	// ִ��
	void Execute();

private:
	Timer::ID NewTimerId();

	void AddTimer(Timer * t);

	int64_t Now();

private:
	Timer::ID _cur_max_timer_id;
	std::list<Timer*> _timer;
	std::vector<Timer*> _add_temp;
	int64_t _min_exec_time;
	bool _executing;
	bool _use_steady_time;
	bool _check_existed_when_new_id;   // ����IDʱ�Ƿ�Ҫ���ID�Ƿ���ڣ�����������󣬻ع鵽0���Ϊtrue(һ������²����е�true��ʱ��)
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
	Timer::ID RegistTimer(int64_t after_msec, typename ObjectTimer<T*>::TimerFunc func)
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