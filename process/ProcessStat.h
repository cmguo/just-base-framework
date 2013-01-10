// ProcessStat.h

#ifndef _FRAMEWORK_PROCESS_PROCESS_STAT_H_
#define _FRAMEWORK_PROCESS_PROCESS_STAT_H_

namespace framework
{
    namespace process
    {

        struct CpuStat
        {
            boost::uint32_t user;
#ifndef BOOST_WINDOWS_API
            boost::uint32_t nice;
#endif
            boost::uint32_t system;
            boost::uint32_t idle;
#ifndef BOOST_WINDOWS_API
            boost::uint32_t iowait;
            boost::uint32_t irq;
            boost::uint32_t softirq;
#endif
            boost::uint32_t total;
        };

        struct SystemStat
        {
            CpuStat cpu;
            std::vector<CpuStat> cpus;
        };

        struct ProcessStat
        {
            enum StateEnum
            {
                running,    // R    Running or runnable (on run queue)
#ifndef BOOST_WINDOWS_API
                sleep1,     // D    Uninterruptible sleep (usually IO)
                sleep2,     // S    Interruptible sleep (waiting for an event to complete)
                stopped,    // T    Stopped, either by a job control signal or because it is being traced.
                zombie,     // Z    Defunct ("zombie") process, terminated but not reaped by its parent.
#endif
                dead,       // X    dead (should never be seen)
            };

            static StateEnum const char_to_state[26]; // start from "A"

            int pid;            // ����(�������������̣����߳�)��
            std::string name;   // Ӧ�ó�������������
            StateEnum state;    // �����״̬
#ifndef BOOST_WINDOWS_API
            int ppid;           // ������ID
            int pgid;           // �߳����
            int sid;            // ���������ڵĻỰ��ID
            int tty_nr;         // �������tty�ն˵��豸��
            int tty_pgrp;       // �ն˵Ľ������
            boost::uint32_t flags;          // ���̱�־λ
            boost::uint32_t min_flt;        // ��������Ҫ��Ӳ�̿����ݶ�������ȱҳ����ȱҳ���Ĵ���
            boost::uint32_t cmin_flt;       // �ۼƵĸ���������е�waited-for�������������Ĵ�ȱҳ�Ĵ���Ŀ
            boost::uint32_t maj_flt;        // ��������Ҫ��Ӳ�̿����ݶ�������ȱҳ����ȱҳ���Ĵ���
            boost::uint32_t cmaj_flt;       // �ۼƵĸ���������е�waited-for����������������ȱҳ�Ĵ���Ŀ
#endif
            boost::uint32_t utime;          // ���������û�̬���е�ʱ�䣬��λΪjiffies
            boost::uint32_t stime;          // �������ں���̬���е�ʱ�䣬��λΪjiffies
#ifndef BOOST_WINDOWS_API
            boost::uint32_t cutime;         // �ۼƵĸ���������е�waited-for�����������û�̬���е�ʱ�䣬��λΪjiffies
            boost::uint32_t cstime;         // �ۼƵĸ���������е�waited-for���������ں���̬���е�ʱ�䣬��λΪjiffies
            boost::uint32_t priority;       // ����Ķ�̬���ȼ�
            boost::uint32_t nice;           // ����ľ�̬���ȼ�
            boost::uint32_t num_threads;    // ���������ڵ��߳������̵߳ĸ���
            boost::uint32_t it_real_value;  // ���ڼ�ʱ������µ���һ�� SIGALRM ���ͽ��̵�ʱ�ӣ��� jiffy Ϊ��λ.
            boost::uint32_t start_time;     // ������������ʱ�䣬��λΪjiffies
            boost::uint32_t vsize;          // ������������ַ�ռ��С��page��
            boost::uint32_t rss;            // ������ǰפ��������ַ�ռ�Ĵ�С(page)
            boost::uint32_t rlim;           // ��������פ��������ַ�ռ�����ֵ��bytes��
            boost::uint32_t start_code;     // �������������ַ�ռ�Ĵ���ε���ʼ��ַ
            boost::uint32_t end_code;       // �������������ַ�ռ�Ĵ���εĽ�����ַ
            boost::uint32_t start_stack;    // �������������ַ�ռ��ջ�Ľ�����ַ
            boost::uint32_t kstkesp;        // esp(32 λ��ջָ��) �ĵ�ǰֵ, ���ڽ��̵��ں˶�ջҳ�õ���һ��.
            boost::uint32_t kstkeip;        // ָ��Ҫִ�е�ָ���ָ��, EIP(32 λָ��ָ��)�ĵ�ǰֵ.
            boost::uint32_t pendingsig;     // �������źŵ�λͼ����¼���͸����̵���ͨ�ź�
            boost::uint32_t block_sig;      // �����źŵ�λͼ
            boost::uint32_t sigign;         // ���Ե��źŵ�λͼ
            boost::uint32_t sigcatch;       // ��������źŵ�λͼ
            boost::uint32_t wchan;          // ����ý�����˯��״̬����ֵ�������ȵĵ��õ�
            boost::uint32_t nswap;          // ��swapped��ҳ������ǰû��
            boost::uint32_t cnswap;         // �����ӽ��̱�swapped��ҳ���ĺͣ���ǰû��
            boost::uint32_t exit_signal;    // �ý��̽���ʱ���򸸽��������͵��ź�
            boost::uint32_t task_cpu;       // �������ĸ�CPU��
            boost::uint32_t task_priority;  // ʵʱ���̵�������ȼ���
            boost::uint32_t task_policy;    // ���̵ĵ��Ȳ��ԣ�0=��ʵʱ���̣�1=FIFOʵʱ���̣�2=RRʵʱ���� 
#endif
        };

        struct ProcessStatM
        {
            boost::uint32_t size;       // (pages) ���������ַ�ռ�Ĵ�С VmSize/4
            boost::uint32_t resident;   // (pages) Ӧ�ó�������ʹ�õ������ڴ�Ĵ�С VmRSS/4
#ifndef BOOST_WINDOWS_API
            boost::uint32_t shared;     // (pages) ����ҳ�� 0
            boost::uint32_t trs;        // (pages) ������ӵ�еĿ�ִ�������ڴ�Ĵ�С VmExe/4
            boost::uint32_t lrs;        // (pages) ��ӳ������������ڴ�ռ�Ŀ�Ĵ�С VmLib/4
            boost::uint32_t drs;        // (pages) �������ݶκ��û�̬��ջ�Ĵ�С ��VmData+ VmStk ��4
            boost::uint32_t dt;         // (pages) 0 
#endif
        };

        boost::system::error_code get_system_stat(
            SystemStat & stat);

        boost::system::error_code get_process_stat(
            int pid, 
            ProcessStat & stat);

        boost::system::error_code get_process_statm(
            int pid, 
            ProcessStatM & statm);


    } // namespace process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_PROCESS_STAT_H_