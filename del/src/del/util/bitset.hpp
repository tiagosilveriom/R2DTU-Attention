#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <type_traits>


namespace del::util
{
	/*
		Bitset with fixed size given at runtime.
		We want to efficiently support creating very many bitsets of the same sizes, so we don't store common state in each instance.
		The code which uses this class is responsible for tracking the necessary common state externally and passing it into the member functions accordingly.
	*/
	template<typename block_type = std::size_t> 
	class bitset
	{
	public:
		static_assert(std::is_unsigned<block_type>::value);
		static_assert(std::is_trivially_copyable<block_type[]>::value);

		/*
			Provides the shared state.
			Should be small enough to pass by value, ensuring local copies in functions which eases vectorization by the compiler.
		*/
		class common_state
		{
			friend class bitset<block_type>;

			static constexpr std::size_t block_size_bytes = sizeof(block_type);
			static constexpr std::size_t block_size_bits = block_size_bytes * 8;

			std::size_t const size;
			std::size_t const num_blocks;
			block_type const excess_mask;

			std::pair<std::size_t, std::size_t> get_value_idx(std::size_t i) const
			{
#if _DEBUG
				if (i >= this->size) throw std::runtime_error("Index out of bounds");
#endif
				return { i / block_size_bits, i - (i / block_size_bits) * this->num_blocks };
			}

		public:
			explicit common_state(std::size_t size) :
				size(size),
				num_blocks(size / block_size_bits + (size % block_size_bits ? 1 : 0)),
				excess_mask(((size % block_size_bits) ? ((block_type)1 << (size % block_size_bits)) : (block_type)0) - 1)
			{
			}
		};

		explicit bitset(common_state const & cs) :
			blocks(std::make_unique<block_type[]>(cs.num_blocks))
		{
			/*
				We assume all operations on bitsets are only performed of bitsets of same size.
				Blocks are zero-initialized.
				We will set and preserve the excess bits in the last block to be 0.
				This invariant is used and maintained by all operations.
			*/
		}

		/*
			Bitsets won't be copyable in the usual way, since they don't contain enough state to take a deep copy.
		*/
		bitset(bitset const &) = delete;
		bitset & operator=(bitset const &) = delete;

		/*
			In place of copy constructor.
		*/
		bitset(common_state const & cs, bitset const & b) :
			blocks(std::make_unique<block_type[]>(cs.num_blocks))
		{
			/* Make sure memcpy copies all blocks entirely (including the excess bits, so they're zeroed). */
			std::memcpy(this->blocks.get(), b.blocks.get(), cs.num_blocks * cs.block_size_bytes);
		}

		/*
			In place of copy assignment.
		*/
		bitset & copy(common_state const & cs, bitset const & b)
		{
			/* Excess bits must already be zeroed; so we could have omitted to copy these. */
			std::memcpy(this->blocks.get(), b.blocks.get(), cs.num_blocks * cs.block_size_bytes);
			return *this;
		}

		bitset(bitset && b) noexcept :
			blocks(std::move(b.blocks))
		{
			/* Excess bits inherently zeroed. */
		}

		bitset & operator=(bitset && b) noexcept
		{
			/* Excess bits inherently zeroed. */
			this->blocks = std::move(b.blocks);
			return *this;
		}

		~bitset() = default;

		bitset & set(common_state const & cs, std::size_t i, bool value)
		{
			auto[block_idx, bit_idx] = cs.get_value_idx(i); //Getting Block and Bit Index
			if (value) this->blocks[block_idx] |= (static_cast<block_type>(1) <<  bit_idx); //sets the bit at position bit_idx within the block at block_idx to 1.
			else this->blocks[block_idx] &= ~(static_cast<block_type>(1) << bit_idx); //clears the bit at position bit_idx within the block at block_idx, setting it to 0.
			return *this;
		}

		bitset & inplace_intersection(common_state const & cs, bitset const & b)
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				this->blocks[i] &= b.blocks[i];
			}
			return *this;
		}

		bitset & inplace_union(common_state const & cs, bitset const & b)
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				this->blocks[i] |= b.blocks[i];
			}
			return *this;
		}

		bitset & inplace_symmetric_difference(common_state const & cs, bitset const & b)
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				this->blocks[i] ^= b.blocks[i];
			}
			return *this;
		}

		bitset & inplace_difference(common_state const & cs, bitset const & b)
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				this->blocks[i] &= ~b.blocks[i];
			}
			return *this;
		}

		bitset & flip(common_state const & cs)
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				this->blocks[i] = ~this->blocks[i];
			}
			this->blocks[cs.num_blocks - 1] &= cs.excess_mask;
			return *this;
		}

		bitset & clear(common_state const & cs)
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				this->blocks[i] = 0;
			}
			return *this;
		}

		bool get(common_state const & cs, std::size_t i) const
		{
			auto[block_idx, bit_idx] = cs.get_value_idx(i);
			return (this->blocks[block_idx] >> bit_idx) & static_cast<block_type>(1);
		}

		bool is_subset_of(common_state const & cs, bitset const & b) const
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				if ((this->blocks[i] | b.blocks[i]) != b.blocks[i]) return false;
			}
			return true;
		}

		bool intersects(common_state const & cs, bitset const & b) const
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				if (this->blocks[i] & b.blocks[i]) return true;
			}
			return false;
		}

		bool none(common_state const & cs) const
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				if (this->blocks[i]) return false;
			}
			return true;
		}

		bool equals(common_state const & cs, bitset const & b) const
		{
			// TODO: Verify that this can be auto-vectorized. Const common_state might not be enough? Might have to take local copy of num_blocks?
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				if (this->blocks[i] != b.blocks[i]) return false;
			}
			return true;
		}

		bool not_equals(common_state const & cs, bitset const & b) const
		{
			return !this->equals(cs, b);
		}

		// Only for diagnostics.
		block_type get_first_block() const
		{
			return this->blocks[0];
		}

		// Only for diagnostics.
		std::size_t get_hash(common_state const & cs) const {
			size_t h = 0;
			for (size_t i = 0; i < cs.num_blocks; ++i)
			{
				// From boost::hash_combine:
				h ^= this->blocks[i] + 0x9e3779b9 + (h << 6) + (h >> 2);
			}
			return h;
		}

	private:
		std::unique_ptr<block_type[]> blocks;
	};
}
