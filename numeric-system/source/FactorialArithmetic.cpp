#include "FactorialArithmetic.h"

namespace numsystem {
	namespace {
		using FA = internal::FactorAccess;
		using BNO = impl::BigNumberOperations;
		using OverflowOps = impl::OverflowAwareOps;
	}

	FactorialArithmetic::FactorialArithmetic(std::string_view value) {
		if (!BNO::is_integral_valid_string(value)) {
			throw std::invalid_argument("Invalid input string for integral string disability error. Value: " + std::string(value));
		}
		if (value.front() == '-') {
			_storage.sign(true);
			value.remove_prefix(1);
		}
		if (value.size() == 1 && value.front() == '0') {
			_storage.push_back(0);
			return;
		}

		// Копируем value в std::string для безопасной модификации
		std::string _value(value);
		std::vector<uint64_t> storage{};

		// Процесс преобразования числа в факториальное представление
		// i - это индекс цифры в факториальном представлении (0 для d_0, 1 для d_1 и т.д.)
		for (uint64_t digit_idx = 0; _value != "0"; ++digit_idx) {
			// Делитель для нахождения цифры d_i равен (i + 1)
			uint64_t divisor = digit_idx + 1;

			// Остаток от деления на divisor — это цифра d_{digit_idx}
			auto [quotient, remainder] = BNO::divide_string_by_integral(_value, divisor);
			std::swap(_value, quotient);

			// Добавляем цифру в вектор
			// storage[0] будет d_0, storage[1] будет d_1 и т.д.
			storage.push_back(remainder);

			// _value обновляется частным внутри divide_string_by_integral, 
			// цикл продолжается, пока частное не станет нулем
		}

		BNO::remove_zeros(storage, BNO::TrimMode::Trailing);
		for (size_t idx = 0; idx < storage.size(); idx++) {
			FA::put(_storage, idx, storage[idx]);
		}
	}
	int FactorialArithmetic::compare(const FactorialArithmetic& other) const noexcept {
		using FA = internal::FactorAccess;
		if (sign() != other.sign()) {
			return sign() ? -1 : 1;
		}

		bool this_is_zero = is_zero();
		bool other_is_zero = other.is_zero();
		if (this_is_zero && other_is_zero) return 0;
		if (this_is_zero && !other_is_zero) return -1;
		if (!this_is_zero && other_is_zero) return 1;

		size_t maxindex = std::max(_storage.value(), other._storage.value());
		for (size_t i = maxindex + 1; i-- > 0;) {
			uint64_t lhs_coeff = FA::extract(_storage, i).value_or(0);
			uint64_t rhs_coeff = FA::extract(other._storage, i).value_or(0);

			if (lhs_coeff < rhs_coeff) {
				return sign() ? 1 : -1;
			}
			if (lhs_coeff > rhs_coeff) {
				return sign() ? -1 : 1;
			}
		}

		return 0;
	}
	
	FactorialArithmetic FactorialArithmetic::add(const FactorialArithmetic& other) const {
		FactorialArithmetic result;
		int carry = 0;
		size_t idx = 0;

		while (true) {
			auto a_opt = FA::extract(_storage, idx);
			auto b_opt = FA::extract(other._storage, idx);

			// Условие выхода: если оба коэффициента отсутствуют и нет переноса
			if (!a_opt.has_value() && !b_opt.has_value() && carry == 0) break;

			uint64_t a = a_opt.value_or(0);
			uint64_t b = b_opt.value_or(0);
			uint64_t base = idx + 1;

			uint64_t sum = a + b + carry;
			carry = 0;

			if (sum >= base) {
				carry = 1;
				sum -= base;
			}

			FA::put(result._storage, idx, sum);
			++idx;
		}
		result.trim_leading_zeros();
		return result;
	}
	FactorialArithmetic FactorialArithmetic::subtract(const FactorialArithmetic& other) const {
		FactorialArithmetic result;
		int64_t borrow = 0;
		size_t idx = 0;

		while (true) {
			auto a_opt = FA::extract(_storage, idx);
			auto b_opt = FA::extract(other._storage, idx);

			// Условие выхода: если обе опции пусты и нет долга
			if (!a_opt.has_value() && !b_opt.has_value() && borrow == 0) break;

			uint64_t a = a_opt.value_or(0);
			uint64_t b = b_opt.value_or(0);
			uint64_t base = idx + 1;

			int64_t diff = static_cast<int64_t>(a) - static_cast<int64_t>(b) - borrow;

			if (diff < 0) {
				diff += base;
				borrow = 1;
			}
			else {
				borrow = 0;
			}
			FA::put(result._storage, idx, static_cast<uint64_t>(diff));

			++idx;
		}

		// Диагностика: если остался долг, значит |lhs| < |rhs|
		if (borrow != 0) {
			FA::put(result._storage, idx, static_cast<uint64_t>(-borrow));
		}
		result.trim_leading_zeros();
		return result;
	}
	
	FactorialArithmetic FactorialArithmetic::multiply(const FactorialArithmetic& other) const {
		// Проверка на ноль (нужно определить метод is_zero() или использовать compare с 0)
		if (is_zero() || other.is_zero()) return FactorialArithmetic(0);

		// Получаем строковые представления и знаки
		std::string a = to_string(*this);
		std::string b = to_string(other);
		bool negA = (!a.empty() && a[0] == '-');
		bool negB = (!b.empty() && b[0] == '-');

		// Убираем знак для работы с абсолютными значениями
		if (negA) a.erase(0, 1);
		if (negB) b.erase(0, 1);
	
		std::string res = BNO::multiply_strings(a, b);

		// Восстанавливаем знак результата
		if (negA ^ negB && res != "0") {
			res.insert(res.begin(), '-');
		}

		return FactorialArithmetic(res);
	}
	FactorialArithmetic FactorialArithmetic::divide(const FactorialArithmetic& other) const	{
		if (is_zero()) return FactorialArithmetic(0);
		if (other.is_zero()) throw std::overflow_error("Division by zero");

		static constexpr char kMINUS = '-';

		std::string a = to_string(*this);
		std::string b = to_string(other);

		bool negA = (!a.empty() && a[0] == kMINUS);
		bool negB = (!b.empty() && b[0] == kMINUS);

		if (negA) a.erase(0, 1);
		if (negB) b.erase(0, 1);

		auto [current, quotient] = BNO::divide_strings(a, b);
		bool negResult = (negA ^ negB) && current != "0";
		if (negResult) current.insert(current.begin(), kMINUS);

		return FactorialArithmetic(current);
	}
	FactorialArithmetic FactorialArithmetic::modulo(const FactorialArithmetic& other) const	{
		if (other.is_zero()) throw std::overflow_error("Division by zero");
		if (is_zero()) return FactorialArithmetic(0);

		FactorialArithmetic quotient = divide(other);
		FactorialArithmetic remainder = *this - (quotient * other);

		// Остаток всегда имеет знак lhs
		remainder.sign(this->sign());
		remainder.trim_leading_zeros();
		return remainder;
	}
		
	bool FactorialArithmetic::is_zero() const noexcept {
		for (size_t idx = 0; idx < FA::MAXINDEX; ++idx) {
			auto digit_opt = FA::extract(_storage, idx);
			if (!digit_opt.has_value()) {
				break; // достигли конца
			}
			if (digit_opt.value() != 0) {
				return false; // нашли ненулевой коэффициент
			}
		}
		return true; // всё нули
	}
	void FactorialArithmetic::trim_leading_zeros() noexcept {
		size_t maxindex = 0;

		// Находим последний индекс, где есть коэффициент
		for (size_t idx = 0; idx < FA::MAXINDEX; ++idx) {
			auto digit_opt = FA::extract(_storage, idx);
			if (!digit_opt.has_value()) {
				break; // достигли конца коэффициентов
			}
			maxindex = idx;
		}

		// Если нет ни одного коэффициента, очищаем весь storage
		if (maxindex == 0 && !_storage.empty()) {
			_storage.clear();
			_storage.push_back(0);
			_storage.value(0);
			return;
		}

		// Считаем сколько бит занято для коэффициентов от 0 до max_index включительно
		// Подсчёт количества слов (элементов) в _storage, которые занимают эти биты
		size_t bits_used = FA::total_bits_up_to(maxindex + 1);
		size_t words_used = (bits_used + _storage.VALUE_COUNT_BIT - 1) / _storage.VALUE_COUNT_BIT;

		// Обрезаем _storage до words_used
		if (_storage.size() > words_used) {
			_storage.resize(words_used);		
		}
		_storage.value(maxindex);
	}
	std::string to_string(const FactorialArithmetic& other) {
		if (other.is_zero()) return "0";
		const auto& refdata = other._storage;
		auto multiply_string_by_uint64 = [](std::string_view num_str, uint64_t factor) -> std::string {
			if (factor == 0 || num_str == "0") return "0";
			if (factor == 1) return std::string(num_str); // копируем исходную строку как есть

			std::string result;
			result.reserve(num_str.size() + 20); // немного с запасом

			uint64_t carry = 0;
			for (int i = static_cast<int>(num_str.length()) - 1; i >= 0; --i) {
				uint64_t digit = num_str[i] - '0';
				uint64_t product = digit * factor + carry;
				result += std::to_string(product % 10);
				carry = product / 10;
			}
			while (carry > 0) {
				result += std::to_string(carry % 10);
				carry /= 10;
			}
			std::reverse(result.begin(), result.end());
			return result;
			};
		auto add_strings = [](std::string_view num1_str, std::string_view num2_str) -> std::string {
			if (num1_str == "0") return std::string(num2_str);
			if (num2_str == "0") return std::string(num1_str);

			std::string sum;
			sum.reserve(std::max(num1_str.size(), num2_str.size()) + 1);

			int i = static_cast<int>(num1_str.length()) - 1;
			int j = static_cast<int>(num2_str.length()) - 1;
			int carry = 0;

			while (i >= 0 || j >= 0 || carry) {
				int digit1 = (i >= 0) ? num1_str[i--] - '0' : 0;
				int digit2 = (j >= 0) ? num2_str[j--] - '0' : 0;
				int current_sum = digit1 + digit2 + carry;
				sum += std::to_string(current_sum % 10);
				carry = current_sum / 10;
			}
			std::reverse(sum.begin(), sum.end());
			return sum;
			};

		std::string decimal_sum = "0";
		std::string current_factorial = "1";  // 0! = 1

		for (size_t idx = 0; idx <= refdata.value(); ++idx) {
			auto value_r_opt = FA::extract(refdata, idx);
			if (!value_r_opt.has_value()) break;

			uint64_t coefficient = value_r_opt.value();
			if (coefficient != 0) {
				std::string term_str = multiply_string_by_uint64(current_factorial, coefficient);
				decimal_sum = add_strings(decimal_sum, term_str);
			}

			current_factorial = multiply_string_by_uint64(current_factorial, idx + 1);
		}

		if (other.sign() && decimal_sum != "0") {
			return "-" + decimal_sum;
		}
		return decimal_sum;
	}	
}