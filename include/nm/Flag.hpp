#ifndef NM_FLAG_HPP
#define NM_FLAG_HPP

namespace nm
{
    class Flag
    {
      private:
        bool flag = false;

      public:
        bool operator()()
        {
            bool temporary = flag;
            flag           = false;
            return temporary;
        }

        void set()
        {
            flag = true;
        }
    };
}  // namespace nm

#endif
