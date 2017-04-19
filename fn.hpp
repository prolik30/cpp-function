#ifndef FN_HPP_4363564
#define FN_HPP_4363564

#include <memory>
#include <exception>
#include <algorithm>

namespace fn
{
	using std::unique_ptr;

	class bad_function_call : std::exception
	{
	public:
	    bad_function_call() : exception() {}
	    virtual const char* what() const noexcept { return "Error: bad function call"; }
	    virtual ~bad_function_call() {}
	};

	template <typename noType>
	class function; 

	template <typename returnType, typename ... argsTypes>
	class function <returnType(argsTypes ...)>
	{
	public:
		function() : pFunc_(nullptr) 
		{}

		function(std::nullptr_t n) : pFunc_(nullptr)
		{}

		template <typename funcType>
		function(funcType f) : myCaller_(new free_holder<funcType>(f)), pFunc_(nullptr) 
		{}

		function(returnType (*func)(argsTypes ... args)) : pFunc_(func)
		{}

		returnType operator()(argsTypes ... args) const
		{
			if(!(*this))
				throw bad_function_call();
			if(pFunc_ != nullptr)
				return pFunc_(std::forward<argsTypes>(args) ...);
			else
				return myCaller_->call(std::forward<argsTypes>(args) ...);
		}

		function(const function & other) 
		{
			if(!other)
				myCaller_ = nullptr, pFunc_ = nullptr;
			else if(other.pFunc_ != nullptr)
				myCaller_ = nullptr, pFunc_ = other.pFunc_;
			else
				myCaller_ = other.myCaller_->clone(), pFunc_ = nullptr;
		}

		function(function&& other)
		{
			if(!other)
				myCaller_ = nullptr, pFunc_ = nullptr;
			else if(other.pFunc_ != nullptr)
				myCaller_ = nullptr, pFunc_ = other.pFunc_;
			else
				myCaller_ = std::move(other.myCaller_), pFunc_ = nullptr;
		}

		function & operator =(const function & other)
		{
			function<returnType(argsTypes...)>(other).swap(*this);
			return *this;
		}

		function & operator =(function&& other)
		{ 
			function<returnType(argsTypes...)>(std::forward<function<returnType(argsTypes...)> >(other)).swap(*this); 
			return *this; 
		}

		void swap(function & other)
		{ 
			std::swap(myCaller_, other.myCaller_); 
			std::swap(pFunc_, other.pFunc_); 
		}

		explicit operator bool() const
		{ return (myCaller_ != nullptr || pFunc_ != nullptr); }

	private:
		class holder_base	
		{
		public:
			holder_base() {}
			virtual ~holder_base() {}
			virtual returnType call(argsTypes ... args) = 0;
			virtual unique_ptr<holder_base> clone()    = 0;
		private:
			holder_base(const holder_base &);
			void operator = (const holder_base &);
		};

		template<typename funcType>
		class free_holder : public holder_base
		{
		public:
			free_holder(funcType inFunc) : holder_base(), myFunc(inFunc) {}

			virtual returnType call(argsTypes ... args)
			{ return myFunc(std::forward<argsTypes>(args) ...); }

			virtual unique_ptr<holder_base> clone()
			{ return unique_ptr<holder_base>(new free_holder<funcType>(myFunc)); }
		private:
			funcType myFunc;
		}; 

		unique_ptr<holder_base> myCaller_;
		returnType (*pFunc_) (argsTypes ...);
	};

	template <typename returnType, typename ... argsTypes>
	void swap(function<returnType(argsTypes ...)> &one, function<returnType(argsTypes ...)> &two)
	{ one.swap(two); }
};

#endif
