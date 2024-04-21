# �rvore rubro-negra com persist�ncia parcial

## Requimentos para compila��o
* cmake
    
## Uso
Antes de executar o programa, � necess�rio criar dois arquivos: um para entrada e outro para sa�da. O arquivo de entrada conter� os comandos a serem executados, enquanto o de sa�da conter� os resultados.

## Exemplo de uso (Linux com g++)
1. Crie os arquivos
    ```
    touch input.txt
    touch outpt.txt
    ```
2. Compile o programa
    ```
    cmake -S .
    make
    ```
3. Execute o programa
    ```
    ./RBTreeFileHandler input.txt output.txt
    ```