import pandas as pd
import numpy as np
from pathlib import Path

def reduce_zero_actions(input_file, output_file, max_zero_sequence=10, keep_interval=5):
    """
    减少标签中长时间连续的0,0（不进行操作的数据）
    
    参数:
        input_file: 输入CSV文件路径
        output_file: 输出CSV文件路径
        max_zero_sequence: 允许的最大连续零动作序列长度
        keep_interval: 在长序列中保留数据的间隔
    """
    print(f"正在读取文件: {input_file}")
    
    # 读取CSV文件
    df = pd.read_csv(input_file)
    print(f"原始数据行数: {len(df)}")
    
    # 获取动作列（最后两列）
    action_cols = df.columns[-2:]
    action_x_col = action_cols[0]  # action_x
    use_energy_col = action_cols[1]  # use_energy
    
    print(f"动作列: {action_x_col}, {use_energy_col}")
    
    # 识别零动作（action_x=0 且 use_energy=0）
    zero_actions = (df[action_x_col] == 0) & (df[use_energy_col] == 0)
    
    # 找到连续的零动作序列
    zero_sequences = []
    start_idx = None
    
    for i in range(len(df)):
        if zero_actions.iloc[i]:
            if start_idx is None:
                start_idx = i
        else:
            if start_idx is not None:
                zero_sequences.append((start_idx, i - 1))
                start_idx = None
    
    # 处理最后一个序列
    if start_idx is not None:
        zero_sequences.append((start_idx, len(df) - 1))
    
    print(f"找到 {len(zero_sequences)} 个零动作序列")
    
    # 筛选需要处理的序列（长度超过阈值）
    long_sequences = [(start, end) for start, end in zero_sequences 
                      if (end - start + 1) > max_zero_sequence]
    
    print(f"其中 {len(long_sequences)} 个序列长度超过 {max_zero_sequence}，需要处理")
    
    # 创建要删除的行索引集合
    rows_to_remove = set()
    
    for start, end in long_sequences:
        sequence_length = end - start + 1
        print(f"处理序列 [{start}, {end}]，长度: {sequence_length}")
        
        # 保留序列的开始部分（前max_zero_sequence个）
        keep_end = start + max_zero_sequence - 1
        
        # 在剩余部分中每隔keep_interval保留一个数据点
        for i in range(keep_end + 1, end + 1):
            if (i - keep_end - 1) % keep_interval != 0:  # 不在保留间隔内的行
                rows_to_remove.add(i)
    
    print(f"需要删除的行数: {len(rows_to_remove)}")
    
    # 创建新的数据框（删除选定的行）
    df_filtered = df.drop(index=list(rows_to_remove)).reset_index(drop=True)
    
    print(f"处理后数据行数: {len(df_filtered)}")
    print(f"数据减少比例: {(1 - len(df_filtered) / len(df)) * 100:.2f}%")
    
    # 保存处理后的数据
    df_filtered.to_csv(output_file, index=False)
    print(f"处理后的数据已保存到: {output_file}")
    
    # 统计信息
    original_zeros = zero_actions.sum()
    filtered_zeros = (df_filtered[action_x_col] == 0) & (df_filtered[use_energy_col] == 0)
    filtered_zeros_count = filtered_zeros.sum()
    
    print(f"\n统计信息:")
    print(f"原始零动作数量: {original_zeros}")
    print(f"处理后零动作数量: {filtered_zeros_count}")
    print(f"零动作减少比例: {(1 - filtered_zeros_count / original_zeros) * 100:.2f}%")
    
    return df_filtered

def analyze_zero_distribution(input_file):
    """
    分析零动作的分布情况
    """
    df = pd.read_csv(input_file)
    action_cols = df.columns[-2:]
    action_x_col = action_cols[0]
    use_energy_col = action_cols[1]
    
    zero_actions = (df[action_x_col] == 0) & (df[use_energy_col] == 0)
    
    # 计算连续序列长度
    sequence_lengths = []
    current_length = 0
    
    for is_zero in zero_actions:
        if is_zero:
            current_length += 1
        else:
            if current_length > 0:
                sequence_lengths.append(current_length)
                current_length = 0
    
    if current_length > 0:
        sequence_lengths.append(current_length)
    
    print(f"\n零动作分布分析:")
    print(f"总零动作数量: {zero_actions.sum()}")
    print(f"零动作比例: {zero_actions.mean() * 100:.2f}%")
    print(f"连续序列数量: {len(sequence_lengths)}")
    
    if sequence_lengths:
        print(f"平均序列长度: {np.mean(sequence_lengths):.2f}")
        print(f"最大序列长度: {max(sequence_lengths)}")
        print(f"序列长度分布:")
        
        # 长度分布统计
        length_counts = {}
        for length in sequence_lengths:
            length_counts[length] = length_counts.get(length, 0) + 1
        
        for length in sorted(length_counts.keys()):
            print(f"  长度 {length}: {length_counts[length]} 个序列")

def main():
    # 硬编码的文件路径
    input_file = "data/training_dataset.csv"
    output_file = "data/training_dataset_reduced.csv"
    max_zero_sequence = 10  # 允许的最大连续零动作序列长度
    keep_interval = 5       # 在长序列中保留数据的间隔
    
    print("使用硬编码的文件路径:")
    print(f"输入文件: {input_file}")
    print(f"输出文件: {output_file}")
    print(f"最大连续零动作序列长度: {max_zero_sequence}")
    print(f"保留数据间隔: {keep_interval}")
    print("-" * 50)
    
    # 检查输入文件
    if not Path(input_file).exists():
        print(f"错误: 输入文件不存在: {input_file}")
        return
    
    # 分析数据分布
    analyze_zero_distribution(input_file)
    
    # 处理数据
    reduce_zero_actions(
        input_file, 
        output_file,
        max_zero_sequence,
        keep_interval
    )
    
    # 分析处理后的数据
    print("\n处理后数据分布:")
    analyze_zero_distribution(output_file)

if __name__ == "__main__":
    main()