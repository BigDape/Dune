#include <iostream>
#include <vector>

namespace STLtest{
    inline int const& Max(const int const& a,const int const& b){
        return a>b?a:b;
    }

    template<typename T>
    inline T const& Max(const T const& a,const T const& b){
        return a>b?a:b;
    }

    template<typename T>
    inline T const& Max(const T const& a,const T const& b,const T const& c){
        return a>b?a:b;
    }

    const std::size_t DefaultStackSize=1024;
    template<typename T,std::size_t n=DefaultStackSize>
    class Stack
    {
    public:
        Stack();
        Stack(Stack<T,n> const&);//除了拷贝构造函数，在类模板中如果要使用到自身，那么就需要使用完整的自定义，而不是省略T
        Stack<T>& operator=(Stack<T,n> const&);
        ~Stack();
        void Push(const T const& element);
        void Pop(T& element);
        int Top(T& element) const;
    private:
        std::vector<T> m_Members;
        std::size_t m_Maxsize=n;
    };

    //模板类做特化
    template<>
    class Stack<std::wstring>
    {
    public:
        void setStackSize(const std::size_t n){ m_Maxsize=n;}
        std::size_t CurrentSize() const {return m_Members.size();}
        void Push(const std::wstring const& element);
        void Pop(std::wstring& element);
        int Top(std::wstring& element) const;
    private:
        std::vector<std::wstring> m_Members;
        std::size_t m_Maxsize;
    };

    //模板类的偏特化
    template<typename T1,typename T2> 
    class MyClass{};

    //偏特化
    template<typename T>
    class MyClass<T,T>
    {};

    template<typename T>
    class MyClass<T,int>
    {};

    template<typename T1,typename T2>
    class MyClass<T1*,T2*>
    {};

    //Traits特性
    template<typename T>
    inline T Sigmal(const T const* start,const T const* end)//错误的例子
    {
        T total = T();
        while(start != end) total += *start;
        return total;
    }

    //存在问题，比如char型时，结果的长度是大于char最大长度的。
    //引入traits：T -> association -> characteristic of T -> anothertype -> trait

    template<typename T>
    class SigmaTraits
    {
    public:
        
    };

    template<>
    class SigmaTraits<char>
    {
    public: 
        typedef int ReturnType;
    };

    template<>
    class SigmaTraits<short>
    {
    public: 
        typedef int ReturnType;
    };

    template<>
    class SigmaTraits<int>
    {
    public: 
        typedef long ReturnType;
    };

    template<>
    class SigmaTraits<unsigned int>
    {
    public: 
        typedef unsigned long ReturnType;
    };

    template<>
    class SigmaTraits<float>
    {
    public: 
        typedef double ReturnType;
    };

    template<typename T>
    inline typename SigmaTraits<T>::ReturnType Sigma(const T const* start,const T const* end)
    {
        typedef typename SigmaTraits::ReturnType ReturnType;
        ReturnType s = ReturnType();
        while(start != end) s += *start;
        return s;
    }


    //迭代器，泛化的指针
    template<class _init,class _Ty>
    inline _init find(_init First,_init Last,const _Ty value)
    {
        for(;First!=Last;++First)
            if(*First==val) break;
        return (First);
    }

}