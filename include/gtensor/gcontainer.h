
#ifndef GTENSOR_GTENSOR_BASE_H
#define GTENSOR_GTENSOR_BASE_H

#include "defs.h"
#include "expression.h"
#include "gstrided.h"
#include "helper.h"

namespace gt
{

// ======================================================================
// gcontainer

template <typename D>
struct gtensor_inner_types;

template <typename D>
class gcontainer : public gstrided<D>
{
public:
  using derived_type = D;
  using base_type = gstrided<D>;
  using inner_types = gtensor_inner_types<D>;
  using inner_storage_type = typename inner_types::storage_type;
  using storage_type = std::decay_t<inner_storage_type>;

  using typename base_type::const_reference;
  using typename base_type::reference;
  using typename base_type::value_type;
  using pointer = typename storage_type::pointer;
  using const_pointer = typename storage_type::const_pointer;
  using typename base_type::shape_type;
  using typename base_type::strides_type;

  using base_type::derived;

  using base_type::base_type;

  template <typename E>
  D& operator=(const expression<E>& e);

  void resize(const shape_type& shape);

  template <typename... Args>
  GT_INLINE const_reference operator()(Args&&... args) const;
  template <typename... Args>
  GT_INLINE reference operator()(Args&&... args);

  GT_INLINE const_reference data_access(size_type i) const;
  GT_INLINE reference data_access(size_type i);

  GT_INLINE const_pointer data() const;
  GT_INLINE pointer data();

  GT_INLINE const storage_type& storage() const;
  GT_INLINE storage_type& storage();
};

// ----------------------------------------------------------------------
// gcontainer implementation

template <typename D>
template <typename E>
inline D& gcontainer<D>::operator=(const expression<E>& e)
{
  resize(e.derived().shape());
  assign(derived(), e.derived());
  return derived();
}

template <typename D>
inline void gcontainer<D>::resize(const shape_type& shape)
{
  this->shape_ = shape;
  this->strides_ = calc_strides(shape);
  storage().resize(calc_size(shape));
}

#pragma nv_exec_check_disable
template <typename D>
GT_INLINE auto gcontainer<D>::data() const -> const_pointer
{
  return storage().data();
}

#pragma nv_exec_check_disable
template <typename D>
GT_INLINE auto gcontainer<D>::data() -> pointer
{
  return storage().data();
}

template <typename D>
template <typename... Args>
GT_INLINE auto gcontainer<D>::operator()(Args&&... args) const
  -> const_reference
{
  return data_access(base_type::index(std::forward<Args>(args)...));
}

template <typename D>
template <typename... Args>
GT_INLINE auto gcontainer<D>::operator()(Args&&... args) -> reference
{
  return data_access(base_type::index(std::forward<Args>(args)...));
}

template <typename D>
GT_INLINE auto gcontainer<D>::data_access(size_type i) const -> const_reference
{
  return derived().data_access_impl(i);
}

template <typename D>
GT_INLINE auto gcontainer<D>::data_access(size_type i) -> reference
{
  return derived().data_access_impl(i);
}

template <typename D>
GT_INLINE auto gcontainer<D>::storage() const -> const storage_type&
{
  return derived().storage_impl();
}

template <typename D>
GT_INLINE auto gcontainer<D>::storage() -> storage_type&
{
  return derived().storage_impl();
}

} // namespace gt

#endif
