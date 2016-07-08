
#include <algorithm>
#include "Timer.h"
#include "TimeHelper.h"

using namespace sframe;

TimerManager::~TimerManager()
{
	for (int i = 0; i < kAllTimerGroupsNumber; i++)
	{
		Timer * t = _timers_groups[i].timer_head;
		while (t)
		{
			Timer * next = t->GetNext();
			delete t;
			t = next;
		}
	}
}

// ע����ͨ��ʱ��
// after_msec: ���ٺ����ִ��
TimerHandle TimerManager::RegistNormalTimer(int64_t after_msec, NormalTimer::TimerFunc func)
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

	NormalTimer * t = new  NormalTimer(func);
	t->SetExecTime(now + after_msec);
	AddTimer(t);

	return t->GetHandle();
}

// ɾ����ʱ��
void TimerManager::DeleteTimer(TimerHandle timer_handle)
{
	if (!timer_handle || !timer_handle->IsAlive())
	{
		return;
	}

	Timer * timer = timer_handle->GetTimerPtr();
	if (timer == nullptr)
	{
		assert(false);
		return;
	}

	// ����ɾ����ǰ����ִ�е�timer
	if (timer == _cur_exec_timer)
	{
		return;
	}

	int32_t group = timer_handle->GetGroup();
	if (group >= 0 && group < kAllTimerGroupsNumber)
	{
		_timers_groups[group].DeleteTimer(timer);
		delete timer;
	}
	else
	{
		auto it = std::find(_add_timer_cache.begin(), _add_timer_cache.end(), timer);
		if (it == _add_timer_cache.end())
		{
			// û����ִ�����У��϶���cache�У�û���ҵ�˵���߼��д���
			assert(false);
			return;
		}

		_add_timer_cache.erase(it);
		delete timer;
	}
}

// ִ��
void TimerManager::Execute()
{
	if (_cur_group_change_time <= 0)
	{
		return;
	}

	// ��λticks
	int64_t cur_time = Now();
	int64_t pass_time = cur_time - _cur_group_change_time;
	if (pass_time < 0)
	{
		assert(false);
		return;
	}

	// ��������
	int64_t pass_group = pass_time / (kTimeSpanOneGroup * 1000);
	int64_t old = pass_group;
	// ִ��
	while (true)
	{
		TimerGroup * group = &_timers_groups[_cur_group];
		if (group->min_exec_time > 0 && cur_time >= group->min_exec_time)
		{
			assert(!group->IsEmpty());

			group->min_exec_time = 0;
			_cur_exec_timer = group->timer_head;
			while (_cur_exec_timer)
			{
				int64_t exec_time = _cur_exec_timer->GetExecTime();
				if (cur_time >= exec_time)
				{
					int64_t after = _cur_exec_timer->Invoke();
					// ��ȡ��һ�ڵ㣬��ɾ����ǰ�ڵ�
					Timer * next_timer = _cur_exec_timer->GetNext();
					group->DeleteTimer(_cur_exec_timer);
					// �Ƿ�Ҫ����ִ��
					if (after >= 0)
					{
						_cur_exec_timer->SetExecTime(cur_time + after);
						_add_timer_cache.push_back(_cur_exec_timer);
					}
					else
					{
						delete _cur_exec_timer;
					}

					_cur_exec_timer = next_timer;
					continue;
				}

				if (group->min_exec_time == 0 || group->min_exec_time > exec_time)
				{
					group->min_exec_time = exec_time;
				}
				_cur_exec_timer = _cur_exec_timer->GetNext();
			}
		}

		if (pass_group <= 0)
		{
			break;
		}

		_cur_group_change_time += (kTimeSpanOneGroup * 1000);
		pass_group--;
		_cur_group++;
		if (_cur_group >= kAllTimerGroupsNumber)
		{
			_cur_group = 0;
		}
	}

	if (!_add_timer_cache.empty())
	{
		for (Timer * t : _add_timer_cache)
		{
			AddTimer(t);
		}
		_add_timer_cache.clear();
	}
}

void TimerManager::AddTimer(Timer * t)
{
	if (_cur_exec_timer)
	{
		_add_timer_cache.push_back(t);
		return;
	}

	if (_cur_group_change_time <= 0)
	{
		assert(false);
		return;
	}

	int64_t exec_time = t->GetExecTime();
	int64_t pass_millisec = exec_time - _cur_group_change_time;
	if (pass_millisec < 0)
	{
		assert(false);
		return;
	}

	int32_t group_index = ((pass_millisec / (kTimeSpanOneGroup * 1000)) + _cur_group) % kAllTimerGroupsNumber;
	t->GetHandle()->SetGroup(group_index);
	_timers_groups[group_index].AddTimer(t);
	if (_timers_groups[group_index].min_exec_time <= 0 || exec_time < _timers_groups[group_index].min_exec_time)
	{
		_timers_groups[group_index].min_exec_time = exec_time;
	}
}

int64_t TimerManager::Now()
{
	return TimeHelper::GetSteadyMiliseconds();
}
