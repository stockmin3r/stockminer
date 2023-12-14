import pandas as pd


def convert_csv_to_parquet(csv_file_path, parquet_file_path):
    # Read the CSV file using pandas
    df = pd.read_csv(csv_file_path)

    # Convert the DataFrame to Parquet format
    df.to_parquet(parquet_file_path)


csv_file = 'data.csv'
parquet_file = 'output.parquet'

convert_csv_to_parquet(csv_file, parquet_file)
