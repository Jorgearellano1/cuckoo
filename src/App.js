import React, { useState } from 'react';
import './App.css';

const App = () => {
    const [key, setKey] = useState('');
    const [message, setMessage] = useState('');
    const [table1, setTable1] = useState([]);
    const [table2, setTable2] = useState([]);
    const [menuOption, setMenuOption] = useState('1');

    const handleInputChange = (e) => {
        setKey(e.target.value);
    };

    const handleMenuChange = (e) => {
        setMenuOption(e.target.value);
    };

    const handleOperation = async () => {
        let endpoint;
        switch (menuOption) {
            case '1':
                endpoint = '/insert';
                break;
            case '2':
                endpoint = '/search';
                break;
            case '3':
                endpoint = '/delete';
                break;
            case '4':
                endpoint = '/print';
                break;
            default:
                setMessage('Opción no válida');
                return;
        }

        try {
            const response = await fetch(`http://localhost:8080${endpoint}?key=${key}`);
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            const data = await response.text();

            if (menuOption === '4') {
                const tables = data.split("\n");
                setTable1(tables[0].trim().split(/\s+/));
                setTable2(tables[1].trim().split(/\s+/));
            } else {
                setMessage(data);
            }
        } catch (error) {
            console.error("Hubo un error:", error);
            setMessage(`Error al realizar la operación: ${error.message}`);
        }
    };

    const renderTable = (table, title) => (
        <div className="hash-table">
            <h2>{title}</h2>
            <table>
                <tbody>
                {table.map((item, index) => (
                    <tr key={index}>
                        <td>{item}</td>
                    </tr>
                ))}
                </tbody>
            </table>
        </div>
    );

    return (
        <div className="app-container">
            <h1>Cuckoo Hashing Demo</h1>
            <div className="controls-container">
                <select value={menuOption} onChange={handleMenuChange}>
                    <option value="1">Insertar un elemento</option>
                    <option value="2">Buscar un elemento</option>
                    <option value="3">Eliminar un elemento</option>
                    <option value="4">Imprimir la tabla</option>
                </select>
                <input type="number" value={key} onChange={handleInputChange} />
                <button onClick={handleOperation}>Ejecutar</button>
            </div>
            <p>Respuesta: {message}</p>
            {menuOption === '4' && (
                <div className="final-hash-tables">
                    <h3>Tablas hash finales:</h3>
                    <div className="tables-container">
                        {renderTable(table1, 'Hash Izquierda')}
                        {renderTable(table2, 'Hash Derecha')}
                    </div>
                </div>
            )}
        </div>
    );
};

export default App;
