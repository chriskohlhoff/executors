#include <experimental/memory>

struct uses_char_allocator
{
  typedef std::allocator<char> allocator_type;

  allocator_type get_allocator() const noexcept
  {
    return allocator_type();
  }
};

template <class T>
class my_allocator : public std::allocator<T>
{
};

int main()
{
  std::allocator<void> a1 = std::experimental::get_associated_allocator([]{});
  std::allocator<char> a2 = std::experimental::get_associated_allocator(uses_char_allocator());
  my_allocator<double> a3 = std::experimental::get_associated_allocator([]{}, my_allocator<double>());
  std::allocator<char> a4 = std::experimental::get_associated_allocator(uses_char_allocator(), my_allocator<double>());

  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
}
