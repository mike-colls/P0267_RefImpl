#pragma once

#include "xio2d.h"

#include <memory>
#include <functional>
#include <exception>
#include <vector>
#include <string>
#include <algorithm>
#include <system_error>
#include <cstdint>
#include <atomic>
#include <variant>
#include <optional>
#include <cmath>
#include <type_traits>
#include <initializer_list>
#include <cmath>

#ifdef _Filesystem_support_test
#include <filesystem>
#endif

namespace std::experimental::io2d {
	inline namespace v1 {

		// cairo_interpreted_path

		inline cairo_interpreted_path::_Native_handle_type cairo_interpreted_path::_Native_handle() const noexcept {
			return _Cairo_path.get();
		}

		constexpr cairo_interpreted_path::cairo_interpreted_path() noexcept
			: _Cairo_path(nullptr)
		{}

		template <class Allocator>
		inline cairo_interpreted_path::cairo_interpreted_path(const path_builder<Allocator>& pf)
			: _Cairo_path(new cairo_path_t, [](cairo_path_t*) {
			// This deleter intentionally left blank. The dtor will deal with this.
		}) {
			auto processedVec = _Interpret_path_items<Allocator>(pf);
			::std::vector<cairo_path_data_t> vec;
			point_2d lastMoveToPoint;
			for (const auto& val : processedVec) {
				::std::visit([&vec, &lastMoveToPoint](auto&& item) {
					using T = ::std::remove_cv_t<::std::remove_reference_t<decltype(item)>>;
					_Path_group_perform_visit<T>::template _Perform<T>(vec, item, lastMoveToPoint);
				}, val);
			}
			_Cairo_path->num_data = static_cast<int>(vec.size());
			const auto numDataST = vec.size();
			_Cairo_path->data = new cairo_path_data_t[numDataST];
			for (size_t currItemIndex = 0; currItemIndex < numDataST; currItemIndex++) {
				_Cairo_path->data[currItemIndex] = vec[currItemIndex];
			}
			_Cairo_path->status = CAIRO_STATUS_SUCCESS;
		}

		template <class ForwardIterator>
		inline cairo_interpreted_path::cairo_interpreted_path(ForwardIterator first, ForwardIterator last)
			: _Cairo_path(new cairo_path_t, [](cairo_path_t*) {
			// This deleter intentionally left blank. The dtor will deal with this.
		}) {
			auto processedVec = _Interpret_path_items<ForwardIterator>(first, last);
			::std::vector<cairo_path_data_t> vec;
			point_2d lastMoveToPoint;
			for (const auto& val : processedVec) {
				::std::visit([&vec, &lastMoveToPoint](auto&& item) {
					using T = ::std::remove_cv_t<::std::remove_reference_t<decltype(item)>>;
					_Path_group_perform_visit<T>::template _Perform<T>(vec, item, lastMoveToPoint);
				}, val);
			}
			_Cairo_path->num_data = static_cast<int>(vec.size());
			const auto numDataST = vec.size();
			_Cairo_path->data = new cairo_path_data_t[numDataST];
			for (size_t currItemIndex = 0; currItemIndex < numDataST; currItemIndex++) {
				_Cairo_path->data[currItemIndex] = vec[currItemIndex];
			}
			_Cairo_path->status = CAIRO_STATUS_SUCCESS;
		}

		inline cairo_interpreted_path::cairo_interpreted_path(const cairo_interpreted_path& other) noexcept
			: _Cairo_path(other._Cairo_path) {}

		inline cairo_interpreted_path& cairo_interpreted_path::operator=(const cairo_interpreted_path& other) noexcept {
			_Cairo_path = other._Cairo_path;
			return *this;
		}

		inline cairo_interpreted_path::cairo_interpreted_path(cairo_interpreted_path&& other) noexcept
			: _Cairo_path(move(other._Cairo_path)) {
			other._Cairo_path = nullptr;
		}

		inline cairo_interpreted_path& cairo_interpreted_path::operator=(cairo_interpreted_path&& other) noexcept {
			if (this != &other) {
				_Cairo_path = move(other._Cairo_path);
				other._Cairo_path = nullptr;
			}
			return *this;
		}

		inline cairo_interpreted_path::~cairo_interpreted_path() noexcept {
			auto path = _Cairo_path.get();
			if (path != nullptr) {
				if (path->data != nullptr) {
					delete[] path->data;
					path->data = nullptr;
					path->status = CAIRO_STATUS_NULL_POINTER;
				}
				delete path;
				path = nullptr;
				_Cairo_path = nullptr;
			}
		}

		// cairo_brush

		inline cairo_brush::native_handle_type cairo_brush::native_handle() const noexcept {
			return _Brush.get();
		}

		inline cairo_brush::cairo_brush(const rgba_color& color)
			: _Brush()
			, _Image_surface()
			, _Brush_type(brush_type::solid_color) {
			_Brush = shared_ptr<cairo_pattern_t>(cairo_pattern_create_rgba(color.r(), color.g(), color.b(), color.a()), &cairo_pattern_destroy);
			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));
		}

		template <class InputIterator>
		inline cairo_brush::cairo_brush(const point_2d& begin, const point_2d& end, InputIterator first, InputIterator last)
			: _Brush()
			, _Image_surface()
			, _Brush_type(brush_type::linear) {
			_Brush = shared_ptr<cairo_pattern_t>(cairo_pattern_create_linear(begin.x, begin.y, end.x, end.y), &cairo_pattern_destroy);
			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));

			for (auto it = first; it != last; ++it) {
				auto stop = *it;
				cairo_pattern_add_color_stop_rgba(_Brush.get(), stop.offset(), stop.color().r(), stop.color().g(), stop.color().b(), stop.color().a());
			}
			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));
		}

		inline cairo_brush::cairo_brush(const point_2d& begin, const point_2d& end, ::std::initializer_list<gradient_stop> il)
			: _Brush()
			, _Image_surface()
			, _Brush_type(brush_type::linear) {
			_Brush = shared_ptr<cairo_pattern_t>(cairo_pattern_create_linear(begin.x, begin.y, end.x, end.y), &cairo_pattern_destroy);
			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));

			for (const gradient_stop& stop : il) {
				cairo_pattern_add_color_stop_rgba(_Brush.get(), stop.offset(), stop.color().r(), stop.color().g(), stop.color().b(), stop.color().a());
			}

			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));
		}

		template <class InputIterator>
		inline cairo_brush::cairo_brush(const circle& start, const circle& end, InputIterator first, InputIterator last)
			: _Brush()
			, _Image_surface()
			, _Brush_type(brush_type::radial) {
			_Brush = shared_ptr<cairo_pattern_t>(cairo_pattern_create_radial(start.center().x, start.center().y, start.radius(), end.center().x, end.center().y, end.radius()), &cairo_pattern_destroy);
			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));
			for (auto it = first; it != last; ++it) {
				auto stop = *it;
				cairo_pattern_add_color_stop_rgba(_Brush.get(), stop.offset(), stop.color().r(), stop.color().g(), stop.color().b(), stop.color().a());
			}
			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));
		}

		inline cairo_brush::cairo_brush(const circle& start, const circle& end, ::std::initializer_list<gradient_stop> il)
			: _Brush()
			, _Image_surface()
			, _Brush_type(brush_type::radial) {
			_Brush = shared_ptr<cairo_pattern_t>(cairo_pattern_create_radial(start.center().x, start.center().y, start.radius(), end.center().x, end.center().y, end.radius()), &cairo_pattern_destroy);
			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));

			for (const gradient_stop& stop : il) {
				cairo_pattern_add_color_stop_rgba(_Brush.get(), stop.offset(), stop.color().r(), stop.color().g(), stop.color().b(), stop.color().a());
			}

			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));
		}

		inline cairo_brush::cairo_brush(image_surface&& img)
			: _Brush()
			, _Image_surface(make_shared<image_surface>(::std::move(img)))
			, _Brush_type(brush_type::surface) {
			_Brush = shared_ptr<cairo_pattern_t>(cairo_pattern_create_for_surface(_Image_surface.get()->native_handle().csfce), &cairo_pattern_destroy);
			_Throw_if_failed_cairo_status_t(cairo_pattern_status(_Brush.get()));
		}

		inline brush_type cairo_brush::type() const noexcept {
			return _Brush_type;
		}

	}
}