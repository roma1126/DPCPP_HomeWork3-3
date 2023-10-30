#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>

enum Type {
	warning,
	error,
	fatal_error,
	unknown_message
};

class LogMessage {
public:
	LogMessage(Type t, std::string& s) : t_(t), s_(s) {};
	Type type() const 
	{
		return t_;
	};
	const std::string& message() const 
	{
		return s_;
	};
private:
	Type t_;
	std::string& s_;
};

class Handler {
public:
	virtual void handling(LogMessage lm) = 0;
	virtual ~Handler() {};
};

class Chain : public Handler {
private:
	Chain* next_chain;
public:
	Chain(Chain* c) : next_chain(c) {};

	void handling(LogMessage lm) override 
	{
		if (next_chain)
			next_chain->handling(lm);
	}
	Chain* addNext(Chain* c)
	{
		if (next_chain)
			addNext(c);
		else
			next_chain = c;
		return next_chain;
	}
};

class Warning : public Chain {
public:
	Warning(Chain* c = nullptr) : Chain(c) {};
	void handling(LogMessage lm) override
 {
		if (lm.type() == warning)
			std::cout << "Warning: " << lm.message() << std::endl;
		else
			Chain::handling(lm);
	}
};

class Error : public Chain {
public:
	Error(std::ofstream& f, Chain* c = nullptr) : file_(f), Chain(c) {};

	void handling(LogMessage lm) override 
	{
		if (lm.type() == error)
			if (file_.is_open()) 
			{
				file_ << "Error: " << lm.message() << std::endl;
			}
			else
				std::cout << "File not open" << std::endl;
		else
			Chain::handling(lm);
	}
private:
	std::ofstream& file_;
};

class FatalError : public Chain {
public:
	FatalError(Chain* c = nullptr) : Chain(c) {};
	void handling(LogMessage lm) override 
	{
		if (lm.type() == fatal_error)
			throw std::runtime_error{ "Fatal error: " + lm.message() };
		else
			Chain::handling(lm);
	}
};

class UncnownMessage : public Chain {
public:
	UncnownMessage(Chain* c = nullptr) : Chain(c) {};
	void handling(LogMessage lm) override 
	{
		if (lm.type() != error && lm.type() != warning && lm.type() != fatal_error)
			throw std::runtime_error{ "Uncnown message: " + lm.message() };
	}
};



int main(int argc, char** argv) {

	std::ofstream file("file.txt");
	std::string message = "AAAAAAAAAA";
	LogMessage lm(unknown_message, message);

	Warning w;
	Error e(file);
	FatalError fe;
	UncnownMessage um;

	try 
	{
		w.addNext(&e)->addNext(&fe)->addNext(&um);
		w.handling(lm);
	}
	catch (const std::runtime_error& e) 
	{
		std::cout << e.what();
	}

	return 0;
}